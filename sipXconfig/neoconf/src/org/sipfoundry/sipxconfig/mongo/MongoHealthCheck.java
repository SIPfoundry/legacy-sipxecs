/**
 * Copyright (c) 2013 eZuce, Inc. All rights reserved.
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

import java.io.IOException;
import java.util.Collection;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.health.HealthCheckFailure;
import org.sipfoundry.sipxconfig.health.HealthCheckProvider;
import org.sipfoundry.sipxconfig.health.HealthManager;

public class MongoHealthCheck implements HealthCheckProvider {
    private static final Log LOG = LogFactory.getLog(MongoHealthCheck.class);
    private MongoManager m_mongoManager;
    private MongoConfig m_mongoConfig;

    @Override
    public void checkHealth(HealthManager manager) {
        manager.startCheck("Checking MongoDB cluster status");
        try {
            // XX-10812 - If you yum update you machine and config format has changed, you'll
            // get error in job status table because mongodb-status command will fail, so we
            // refresh model here just in case before attempting status check
            m_mongoConfig.writeGlobalModel();
        } catch (IOException e) {
            LOG.error("Could not check mongo health", e);
            throw new HealthCheckFailure(e.getMessage());
        }

        MongoReplSetManager globalManager = m_mongoManager.getGlobalManager();
        MongoMeta meta = globalManager.getMeta();
        manager.pass();
        for (String server : meta.getServers()) {
            manager.startCheck(format("Checking MongoDB server %s status", server));
            String err = null;
            Collection<String> actions = meta.getRequiredActions(server);
            if (actions != null) {
                for (String action : actions) {
                    if (action.equals("SET_MEMBER_META")) {
                        // XX-10812 This can happen because database was rebuilt using straight
                        // mongo operations from CLI, or upgrade from any version prior
                        // to 4.6 update 9. Required action can safely be carried out without
                        // prompting user unless there was an error.
                        globalManager.takeAction(meta.getPrimary().getHostPort(), server, action);
                    } else {
                        err = format("Server %s required action %s", server, action);
                        break;
                    }
                }
            }
            if (err == null) {
                manager.pass();
            } else {
                manager.fail(err);
            }
        }
    }

    public void setMongoManager(MongoManager mongoManager) {
        m_mongoManager = mongoManager;
    }

    public void setMongoConfig(MongoConfig mongoConfig) {
        m_mongoConfig = mongoConfig;
    }
}
