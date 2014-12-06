/**
 *
 *
 * Copyright (c) 2014 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.openfire.plugin.job;

import static org.sipfoundry.commons.mongo.MongoConstants.DESCR;
import static org.sipfoundry.commons.mongo.MongoConstants.EMAIL;
import static org.sipfoundry.commons.mongo.MongoConstants.IM_DISPLAY_NAME;
import static org.sipfoundry.commons.mongo.MongoConstants.IM_ENABLED;
import static org.sipfoundry.commons.mongo.MongoConstants.IM_GROUP;
import static org.sipfoundry.commons.mongo.MongoConstants.IM_ID;
import static org.sipfoundry.commons.mongo.MongoConstants.UID;

import java.util.ArrayList;
import java.util.List;

import org.apache.commons.lang.StringUtils;
import org.apache.log4j.Logger;
import org.bson.types.ObjectId;
import org.jivesoftware.openfire.XMPPServer;
import org.sipfoundry.commons.userdb.User;
import org.sipfoundry.commons.userdb.UserGroup;
import org.sipfoundry.commons.util.UnfortunateLackOfSpringSupportFactory;
import org.sipfoundry.openfire.plugin.job.group.GroupDeleteJob;
import org.sipfoundry.openfire.plugin.job.group.GroupUpdateJob;
import org.sipfoundry.openfire.plugin.job.user.UserDeleteJob;
import org.sipfoundry.openfire.plugin.job.user.UserUpdateJob;
import org.sipfoundry.openfire.plugin.job.vcard.VcardUpdateJob;
import org.sipfoundry.openfire.provider.CacheHolder;
import org.sipfoundry.openfire.sync.MongoOperation;
import org.sipfoundry.openfire.sync.job.AbstractJobFactory;
import org.sipfoundry.openfire.sync.job.Job;
import org.xmpp.packet.JID;

import com.mongodb.BasicDBList;
import com.mongodb.BasicDBObject;
import com.mongodb.DB;
import com.mongodb.DBCollection;
import com.mongodb.DBObject;

public class JobFactory extends AbstractJobFactory {
    private static Logger logger = Logger.getLogger(JobFactory.class);
    private static final String COLLECTION_NAME = "entity";

    @Override
    public Job createJob(MongoOperation op, DBObject dbObj, Object id) {
        Job job = null;

        if (id != null) {
            if (id instanceof String) {
                job = createJob(op, dbObj, (String) id);
            } else if (id instanceof Long) {
                job = createJob(op, dbObj, id);
            } else if (id instanceof ObjectId) {
                // we assume all id's that are Mongo ObjectId
                // are avatar operations
                job = createJob(op, dbObj, (ObjectId) id);
            } else {
                logger.warn(String.format("Unknown id type: %s", id.getClass().getName()));
            }
        } else {
            logger.warn("Id was null.");
        }

        return job;
    }

    private static Job createJob(MongoOperation op, DBObject dbObj, String id) {
        Job job = null;

        if (id.startsWith("User")) {
            job = buildUserJob(op, dbObj, id);
        } else if (id.startsWith("Group")) {
            job = buildGroupJob(op, dbObj, id);
        }

        return job;
    }

    private static Job createJob(MongoOperation op, DBObject dbObj, ObjectId id) {
        logger.debug("id: " + id.toString());
        logger.debug("op: " + op);
        Job vcardUpdateJob = null;

        switch (op) {
        case INSERT:
            if (dbObj.containsField("filename")) {
                String filename = dbObj.get("filename").toString();
                if (filename.startsWith("avatar_")) {
                    String userName = StringUtils.substringBetween(filename, "avatar_", ".");
                    logger.debug("avatar insert for user: " + userName);
                    String imid = null;
                    User user = UnfortunateLackOfSpringSupportFactory.getValidUsers().getUser(userName);
                    if (user != null) {
                        imid = user.getJid();
                    }
                    vcardUpdateJob = imid != null ? new VcardUpdateJob(imid, dbObj) : null;
                }
            }
            break;
        }
        return vcardUpdateJob;
    }

    private static Job buildUserJob(MongoOperation op, DBObject dbObj, String id) {
        logger.debug("User job obj: " + dbObj);

        Job userJob = null;
        // get the im name from the update object; if it's not there (it has not been updated),
        // use the cached value
        String userImName = (String) dbObj.get(IM_ID);
        if (userImName == null) {
            userImName = CacheHolder.getUserName(id);
        }
        if (userImName == null) {
            userImName = lookupImId(id);
        }
        
        List<String> groupNames = UnfortunateLackOfSpringSupportFactory.getValidUsers().getImGroupnamesForUser(userImName);
        logger.debug("loadded group names from mongo: " + groupNames);
        if (!StringUtils.isBlank(userImName)) {
            switch (op) {
            case INSERT:
                if (!(Boolean) dbObj.get(IM_ENABLED) == Boolean.TRUE) {
                    break;
                }
                //$FALL-THROUGH$ - only if IM is enabled, do an update for this user
            case UPDATE:
                String oldImName = CacheHolder.getUserName(id) == null ? userImName : CacheHolder.getUserName(id);
                Boolean imUser = (Boolean) dbObj.get(IM_ENABLED);
                //user is created only if imUser is present in oplog query and is true
                boolean createUser = (imUser != null && imUser);
                // avoid querying the actual value; the value wasn't updated, but the user is
                // cached, therefore it must have IM enabled
                if (imUser == null) {
                    imUser = true;
                }
                String displayName = (String) dbObj.get(IM_DISPLAY_NAME);
                String email = (String) dbObj.get(EMAIL);
                String uid = (String) dbObj.get(UID);
                userJob = new UserUpdateJob(id, userImName, oldImName, imUser, displayName, email, uid, 
                    groupNames == null ? new ArrayList<String>() : groupNames, createUser);
                break;
            case DELETE:
                userJob = new UserDeleteJob(id, userImName);
                break;
            default:
                logger.warn(String.format("Unsupported user operation %s. Ignoring.", op));
                break;
            }
        } else {
            logger.warn("Missing user name for update/delete operation");
        }

        return userJob;
    }

    private static Job buildGroupJob(MongoOperation op, DBObject dbObj, String id) {
        logger.debug("Group job obj: " + dbObj);
        logger.debug("Group job id: " + id);

        Job groupJob = null;
        String groupName = (String) dbObj.get(UID);
        String imGroupProperty = (String) dbObj.get(IM_GROUP);
        //String myBuddyInGroup = (String)dbObj.get(MY_BUDDY_GROUP);
        String oldGroupName = CacheHolder.getGroupName(id); 
        //boolean myBuddyInGroupChange = myBuddyInGroup != null;
        boolean createGroup = (imGroupProperty != null && StringUtils.equals(imGroupProperty, "1"));

        String description = (String) dbObj.get(DESCR);

        String xmppDomain = XMPPServer.getInstance().getServerInfo().getXMPPDomain();
        String imBotName = UnfortunateLackOfSpringSupportFactory.getValidUsers().getImBotName();
        JID imBotJid = new JID(imBotName, xmppDomain, null, false);

        switch (op) {
        case UPDATE:
            // check if group already has im enabled
            if (StringUtils.isNotBlank(oldGroupName)) {
                if (groupName == null) {
                    // group name has not been changed, use the old one
                    groupName = oldGroupName;
                }
                UserGroup group = UnfortunateLackOfSpringSupportFactory.getValidUsers().getImGroup(groupName);
                boolean isImGroup = false;
                boolean isMyBuddyEnabled = false;
                if (group != null) {
                    isImGroup = true;
                    isMyBuddyEnabled = group.isImbotEnabled();
                }
                groupJob = new GroupUpdateJob(id, groupName, oldGroupName, isImGroup, description, isMyBuddyEnabled, imBotJid, createGroup);
            } else {
                // not a known group
                if (imGroupProperty != null) {
                    // im group was enabled
                    groupName = lookupGroupName(id);
                    if (groupName != null) {
                        UserGroup group = UnfortunateLackOfSpringSupportFactory.getValidUsers().getImGroup(groupName);
                        if (group != null) {
                            logger.debug("Add all users in group " + groupName);
                            groupJob = new GroupUpdateJob(id, groupName, groupName, true, group.getDescription(),
                                    group.isImbotEnabled(), imBotJid, createGroup);
                        }
                    }
                }
            }
            break;
        case DELETE:
            if (StringUtils.isNotBlank(oldGroupName)) {
                // an IM group was deleted
                groupJob = new GroupDeleteJob(oldGroupName);
                break;
            }
        default:
            logger.warn(String.format("Unsupported group operation %s. Ignoring.", op));
            break;
        }

        return groupJob;
    }

    private static String lookupImId(String uid) {
        DBObject query = new BasicDBObject("_id", uid);
        DBObject fields = new BasicDBObject("imid", 1);
        DBObject user = getCollection().findOne(query, fields);
        if (user == null) {
            return null;
        }
        return (String) user.get("imid");
    }

    private static String lookupGroupName(String groupId) {
        DBObject query = new BasicDBObject("_id", groupId);
        DBObject fields = new BasicDBObject("uid", 1);
        DBObject user = getCollection().findOne(query, fields);
        if (user == null) {
            return null;
        }
        return (String) user.get("uid");
    }

    private static DBCollection getCollection() {
        DB db = UnfortunateLackOfSpringSupportFactory.getImdb();

        return db.getCollection(COLLECTION_NAME);
    }
}
