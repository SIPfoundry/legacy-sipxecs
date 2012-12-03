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


import static org.junit.Assert.assertEquals;

import java.util.Arrays;
import java.util.Collections;
import java.util.List;

import org.junit.Before;
import org.junit.Test;

public class MongoConfigTest {
    private MongoConfig m_config;
    private List<MongoServer> m_single;
    private List<MongoServer> m_multi;
    
    @Before
    public void setUp() {
        m_config = new MongoConfig();
        MongoServer server1 = new MongoServer();
        server1.setName("one:1");
        server1.setIsServer();
        MongoServer server2 = new MongoServer();
        server2.setName("two:1");
        server2.setIsServer();
        MongoServer server3 = new MongoServer();
        server3.setName("three:1");
        server3.setIsArbiter();
        m_multi = Arrays.asList(server1, server2, server3);
        m_single = Collections.singletonList(server1);
    }
    
    @Test
    public void getConnectionString() {
        assertEquals("sipxecs/one:1", m_config.getConnectionString(m_single, 1));
        assertEquals("sipxecs/one:1,two:1", m_config.getConnectionString(m_multi, 1));
    }

    @Test
    public void getConnectionUrl() {
        assertEquals("mongodb://one:1/?readPreference=nearest", m_config.getConnectionUrl(m_single, 1));
        assertEquals("mongodb://one:1,two:1/?readPreference=nearest", m_config.getConnectionUrl(m_multi, 1));
    }
}
