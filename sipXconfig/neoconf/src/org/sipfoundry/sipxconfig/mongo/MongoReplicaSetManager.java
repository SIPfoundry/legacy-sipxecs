/**
 *
 *
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.mongo;

import static java.lang.String.format;

import java.util.ArrayList;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.bson.BasicBSONObject;
import org.sipfoundry.commons.mongo.MongoUtil;
import org.sipfoundry.commons.mongo.MongoUtil.MongoCommandException;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.commserver.imdb.ReplicationManager;
import org.springframework.data.mongodb.core.MongoTemplate;

import com.mongodb.BasicDBList;
import com.mongodb.BasicDBObject;
import com.mongodb.DB;
import com.mongodb.MongoException;

/**
 * Add/remove servers from Mongo's replica set server list stored in the "local" database on each
 * server.
 * NOTES:
 * We designate the mongo instance that runs with sipxconfig as the preferred master. Among other
 * good reasons, replication manipulation like adding or removing servers can only in the local db
 * on the primary server. Priority > 1 will do this.
 */
public class MongoReplicaSetManager {
    private static final Log LOG = LogFactory.getLog(MongoReplicaSetManager.class);
    private static final String SERVER_ID = "_id";
    private static final String MEMBERS = "members";
    private static final String VOTES = "votes";
    private static final String HOST = "host";
    private static final String CHECK_COMMAND = "rs.config()";
    private static final String INIT_COMMAND = "rs.initiate({\"_id\" : \"sipxecs\", \"version\" : 1, \"members\" : "
            + "[ { \"_id\" : 0, \"host\" : \"%s:%d\", priority : 2 } ] })";
    private static final String ADD_SERVER_COMMAND = "rs.add(\"%s\")";
    private static final String REMOVE_SERVER_COMMAND = "rs.remove(\"%s\")";
    private static final String STEP_DOWN_COMMAND = "rs.stepDown()";
    private static final String RENAME_PRIMARY_COMMAND = "c = rs.config(); c.members[0].host = '%s'; rs.reconfig(c)";
    private static final String VOTER_COMMAND = "c = rs.config(); c.members[%d].votes = %d; rs.reconfig(c)";
    private static final String ADD_ARBITER_COMMAND = "rs.addArb(\"%s\")";
    private static final String GET_STATUS_COMMAND = "rs.status()";
    private static final String COMMAND_FORCE_RECONFIG = "rs.reconfig(%s, { force : true })";
    private MongoTemplate m_localDb;
    private MongoTemplate m_localReplicasetDb;
    private ReplicationManager m_replicationManager;
    private String m_primaryFqdn;
    private ConfigManager m_configManager;

    public void checkState() {
        try {
            BasicBSONObject ret = MongoUtil.runCommand(m_localDb.getDb(), CHECK_COMMAND);
            boolean initialized = false;
            if (ret != null) {
                BasicDBList members = (BasicDBList) ret.get(MEMBERS);
                if (members != null) {
                    BasicDBObject primary = (BasicDBObject) members.get(0);
                    if (primary != null) {
                        initialized = true;
                        String host = primary.getString(HOST);
                        String expected = format(m_primaryFqdn + ':' + MongoSettings.SERVER_PORT);
                        if (!expected.equals(host)) {
                            String rename = format(RENAME_PRIMARY_COMMAND, expected);
                            MongoUtil.runCommand(m_localDb.getDb(), rename);
                        }
                    }
                }
            }

            if (!initialized) {
                initialize();
            }
        } catch (Exception ex) {
            LOG.error("Failed to check replica set state!");
        }

    }

    public void initialize() {
        // cannot use business object because they are not initialized yet
        try {
            LOG.info("initializing mongo replicaset to host " + m_primaryFqdn);
            String cmd = format(INIT_COMMAND, m_primaryFqdn, MongoSettings.SERVER_PORT);
            MongoUtil.runCommand(m_localDb.getDb(), cmd);
            for (int i = 0; i < 12; i++) {
                LOG.info("Testing mongo connection");
                if (m_replicationManager.testDatabaseReady()) {
                    break;
                }
                Thread.sleep(5000);
            }
        } catch (InterruptedException e) {
            LOG.error("Interrupted waiting for mongo primary connection test");
        }
    }

    public List<MongoServer> getMongoServers(boolean readFromLocal, boolean votingDetails) {
        Map<Integer, MongoServer> rsStatus = new LinkedHashMap<Integer, MongoServer>();
        DB ds;
        if (readFromLocal) {
            ds = m_localDb.getDb();
        } else {
            ds = m_localReplicasetDb.getDb();
        }
        String errmsg = "&err.read.localMongo";
        try {
            BasicBSONObject status = MongoUtil.runCommand(ds, GET_STATUS_COMMAND);
            BasicDBList members = (BasicDBList) status.get(MEMBERS);
            for (Object obj : members) {
                BasicDBObject server = (BasicDBObject) obj;
                MongoServer mongoServer = new MongoServer(server);
                rsStatus.put(mongoServer.getId(), mongoServer);
            }
            if (votingDetails) {
                // query rs.config and found out voting / non voting members
                BasicBSONObject config = MongoUtil.runCommand(ds, CHECK_COMMAND);
                BasicDBList configMembers = (BasicDBList) config.get(MEMBERS);
                for (Object configObj : configMembers) {
                    BasicDBObject server = (BasicDBObject) configObj;
                    Integer id = server.getInt(SERVER_ID);
                    MongoServer mongoServer = rsStatus.get(id);
                    if (mongoServer != null && server.containsField(VOTES)) {
                        mongoServer.setVotingMember(server.getInt(VOTES) > 0);
                    }
                }
            }
        } catch (MongoException.Network e) {
            LOG.error(errmsg, e);
            throw new UserException(errmsg);
        } catch (MongoException e) {
            LOG.error(errmsg, e);
            throw new UserException(errmsg);
        }
        return new ArrayList<MongoServer>(rsStatus.values());
    }

    public void addInReplicaSet(String name) {
        String cmd = format(ADD_SERVER_COMMAND, name);
        if (StringUtils.contains(name, String.valueOf(MongoSettings.ARBITER_PORT))) {
            cmd = format(ADD_ARBITER_COMMAND, name);
        }
        try {
            BasicBSONObject result = MongoUtil.runCommand(m_localDb.getDb(), cmd);
            MongoUtil.checkForError(result);
        } catch (MongoCommandException e) {
            LOG.warn("Failed to add in replica set", e);
            throw new UserException("&err.add.rs.member");
        }
        m_configManager.configureEverywhere(MongoManager.FEATURE_ID);
    }

    public void removeFromReplicaSet(String name) {
        String cmd = format(REMOVE_SERVER_COMMAND, name);
        try {
            BasicBSONObject result = MongoUtil.runCommand(m_localDb.getDb(), cmd);
            MongoUtil.checkForError(result);
        } catch (MongoCommandException e) {
            LOG.warn("Failed to remove from replica set", e);
            throw new UserException("&err.remove.rs.member");
        }
        m_configManager.configureEverywhere(MongoManager.FEATURE_ID);
    }

    public void removeVoter(Integer id) {
        modifyVoter(format(VOTER_COMMAND, id, 0));
    }

    public void addVoter(Integer id) {
        modifyVoter(format(VOTER_COMMAND, id, 1));
    }

    private void modifyVoter(String cmd) {
        String changingVoterFailed = "&err.voter.failed";
        try {
            BasicBSONObject result = MongoUtil.runCommand(m_localDb.getDb(), cmd);
            if (result == null) {
                throw new UserException(changingVoterFailed);
            }
            MongoUtil.checkForError(result);
        } catch (MongoCommandException e) {
            LOG.warn("Failed to change voters from replica set", e);
            throw new UserException(changingVoterFailed);
        }
    }

    public void stepDown() {
        try {
            BasicBSONObject result = MongoUtil.runCommand(m_localReplicasetDb.getDb(), STEP_DOWN_COMMAND);
            MongoUtil.checkForError(result);
        } catch (MongoCommandException e) {
            LOG.warn("Failed to step down replica set", e);
            throw new UserException("&err.failed.stepDown");
        }
    }

    public void forceReconfig() {
        try {
            BasicDBObject config = new BasicDBObject();
            config.put(SERVER_ID, "sipxecs");
            config.put("version", 1);
            BasicDBList members = new BasicDBList();
            BasicDBObject primary = new BasicDBObject();
            primary.put(SERVER_ID, 0);
            primary.put(HOST, format("%s:%d", m_primaryFqdn, MongoSettings.SERVER_PORT));
            primary.put("priority", 2);
            members.add(primary);
            config.put(MEMBERS, members);
            String command = format(COMMAND_FORCE_RECONFIG, config.toString());
            BasicBSONObject result = MongoUtil.runCommand(m_localDb.getDb(), command);
            MongoUtil.checkForError(result);
        } catch (MongoCommandException e) {
            LOG.warn("Failed to force replica set reconfiguration", e);
            throw new UserException("&err.failed.forceReconfig");
        }
    }

    public void setLocalDb(MongoTemplate localDb) {
        m_localDb = localDb;
    }

    public void setLocalReplicaSetDb(MongoTemplate localRs) {
        m_localReplicasetDb = localRs;
    }

    public void setReplicationManager(ReplicationManager replicationManager) {
        m_replicationManager = replicationManager;
    }

    public void setPrimaryFqdn(String primaryFqdn) {
        m_primaryFqdn = primaryFqdn;
    }

    public void setConfigManager(ConfigManager manager) {
        m_configManager = manager;
    }
}
