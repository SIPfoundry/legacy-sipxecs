/*
 * Copyright (C) 2012 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.mongo;

import static java.lang.String.format;

import java.net.InetAddress;
import java.net.UnknownHostException;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.bson.BasicBSONObject;
import org.sipfoundry.commons.mongo.MongoUtil;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.feature.FeatureListener;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.feature.GlobalFeature;
import org.sipfoundry.sipxconfig.feature.LocationFeature;
import org.springframework.data.mongodb.core.MongoTemplate;

/**
 * Add/remove servers from Mongo's replica set server list stored in the "local" database on each
 * server.
 */
public class MongoReplicaSetManager implements FeatureListener {
    private static final Log LOG = LogFactory.getLog(MongoReplicaSetManager.class);
    private static final String CHECK_COMMAND = "rs.config()";
    private static final String INIT_COMMAND = "rs.initiate({\"_id\" : \"sipxecs\", \"version\" : 1, \"members\" : "
        + "[ { \"_id\" : 0, \"host\" : \"%s:27017\" } ] })";
    private MongoTemplate m_localDb;

    public void checkState() {
        BasicBSONObject ret = MongoUtil.runCommand(m_localDb.getDb(), CHECK_COMMAND);
        if (ret == null) {
            initialize();
        }
        checkMembers();
    }

    public void initialize() {
        // cannot use business object because they are not initialized yet
        try {
            String fqdn = InetAddress.getLocalHost().getHostName();
            LOG.info("initializing mongo replicaset to host " + fqdn);
            String cmd = format(INIT_COMMAND, fqdn);
            MongoUtil.runCommand(m_localDb.getDb(), cmd);
        } catch (UnknownHostException e) {
            throw new IllegalStateException("Cannot it FQDN to initialize mongo.");
        }
    }

    public void checkMembers() {
        // TODO:
    }

    @Override
    public void enableLocationFeature(FeatureManager manager, FeatureEvent event, LocationFeature feature,
            Location location) {
        if (!feature.equals(MongoManager.FEATURE_ID)) {
            return;
        }
        if (event != FeatureEvent.POST_ENABLE || event != FeatureEvent.POST_DISABLE) {
            return;
        }
        checkMembers();
    }

    @Override
    public void enableGlobalFeature(FeatureManager manager, FeatureEvent event, GlobalFeature feature) {
    }

    public void setLocalDb(MongoTemplate localDb) {
        m_localDb = localDb;
    }
}
