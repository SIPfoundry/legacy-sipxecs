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
import org.sipfoundry.sipxconfig.commserver.Location;

public class MongoConfigTest {
    private MongoConfig m_config;
    private List<Location> m_single;
    private List<Location> m_multi;
    
    @Before
    public void setUp() {
        m_config = new MongoConfig();
        m_multi = Arrays.asList(new Location("one"), new Location("two"));
        m_multi.get(0).setAddress("1.1.1.1");
        m_multi.get(1).setAddress("1.1.1.2");
        m_single = Collections.singletonList(m_multi.get(0));
    }
    
    @Test
    public void getConnectionString() {
        assertEquals("sipxecs/one:1", m_config.getConnectionString(m_single, 1));
        assertEquals("sipxecs/one:1,two:1", m_config.getConnectionString(m_multi, 1));
    }

    @Test
    public void getConnectionUrl() {
        assertEquals("mongodb://one:1/?slaveOk=true", m_config.getConnectionUrl(m_single, 1));
        assertEquals("mongodb://one:1,two:1/?slaveOk=true", m_config.getConnectionUrl(m_multi, 1));
    }
}
