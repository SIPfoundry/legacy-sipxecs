/*
 * Copyright (C) 2012 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
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
        assertEquals("[localhost:27017, remote.example.org:27017]", actualMembers.toString());
        assertEquals("[localhost:27018]", actualArbiters.toString());
    }
    
    public void testMongoMembers() throws Exception {
        Set<String> actualMembers = new HashSet<String>();
        Set<String> actualArbiters = new HashSet<String>();
        m_mongoReplicaSetManager.getMongoMembers(actualMembers, actualArbiters);
        String actual = actualMembers.toString();
        // Should be [hostname:27017]
        assertTrue("Got " + actual, actual.matches("\\[[\\w.]+:27017\\]"));
    }

    public void setMongoReplicaSetManager(MongoReplicaSetManager mongoReplicaSetManager) {
        m_mongoReplicaSetManager = mongoReplicaSetManager;
    }
}
