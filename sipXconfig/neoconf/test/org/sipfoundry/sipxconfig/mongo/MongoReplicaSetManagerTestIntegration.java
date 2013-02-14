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

import java.util.List;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.test.IntegrationTestCase;

public class MongoReplicaSetManagerTestIntegration extends IntegrationTestCase {
    private MongoReplicaSetManager m_mongoReplicaSetManager;
    
    @Override
    public void onSetUpBeforeTransaction() throws Exception {
        super.onSetUpBeforeTransaction();
        clear();
        sql("commserver/SeedLocations.sql");
    }

    public void testGetMongoServers() throws Exception {
        List<MongoServer> servers = m_mongoReplicaSetManager.getMongoServers(true, false);
        for (MongoServer server : servers) {
            assertTrue(StringUtils.contains(server.getName(), ":27017") || StringUtils.contains(server.getName(), ":27018"));
        }
    }

    public void setMongoReplicaSetManager(MongoReplicaSetManager mongoReplicaSetManager) {
        m_mongoReplicaSetManager = mongoReplicaSetManager;
    }
}
