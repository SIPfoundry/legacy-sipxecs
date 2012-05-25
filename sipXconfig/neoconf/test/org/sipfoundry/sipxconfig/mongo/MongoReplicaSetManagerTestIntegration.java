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

import java.util.HashSet;
import java.util.Set;

import org.sipfoundry.sipxconfig.test.IntegrationTestCase;

public class MongoReplicaSetManagerTestIntegration extends IntegrationTestCase {
    private MongoReplicaSetManager m_mongoReplicaSetManager;
    
    @Override
    public void onSetUpBeforeTransaction() throws Exception {
        super.onSetUpBeforeTransaction();
        clear();
        sql("commserver/SeedLocations.sql");
    }

    public void testPostgresMembers() throws Exception {
        sql("mongo/seed_mongo_servers.sql");
        Set<String> actualMembers = new HashSet<String>();
        Set<String> actualArbiters = new HashSet<String>();
        m_mongoReplicaSetManager.getPostgresMembers(actualMembers, actualArbiters);
        assertEquals("[remote.example.org:27017]", actualMembers.toString());
        assertEquals("[localhost:27018]", actualArbiters.toString());
    }
    
    public void testMongoMembers() throws Exception {
        Set<String> actualMembers = new HashSet<String>();
        Set<String> actualArbiters = new HashSet<String>();
        m_mongoReplicaSetManager.getMongoMembers(actualMembers, actualArbiters);
        String actual = actualMembers.toString();
        // Should be [hostname:27017]
        assertTrue("Got " + actual, actual.matches("\\[[\\-\\w.]+:27017\\]"));
    }

    public void setMongoReplicaSetManager(MongoReplicaSetManager mongoReplicaSetManager) {
        m_mongoReplicaSetManager = mongoReplicaSetManager;
    }
}
