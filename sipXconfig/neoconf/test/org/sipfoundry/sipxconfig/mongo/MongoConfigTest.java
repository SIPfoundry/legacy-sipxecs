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
import java.util.List;

import org.junit.Before;
import org.junit.Test;
import org.sipfoundry.sipxconfig.commserver.Location;

public class MongoConfigTest {
    private MongoConfig m_config;
    private Location m_primary;
    private List<Location> m_secondary;
    
    @Before
    public void setUp() {
        m_config = new MongoConfig();
        m_primary = new Location("primary");
        m_primary.setAddress("1.1.1.0");
        m_secondary = Arrays.asList(new Location("one"), new Location("two"));
        m_secondary.get(0).setAddress("1.1.1.1");
        m_secondary.get(1).setAddress("1.1.1.2");
    }
    
    @Test
    public void connString() {
        assertEquals("sipxecs/primary:1", m_config.getConnectionString(m_primary, null, 1, false));
        assertEquals("sipxecs/primary:1,one:1,two:1", m_config.getConnectionString(m_primary, m_secondary, 1, false));
    }

    @Test
    public void connUrl() {
        assertEquals("mongodb://primary:1/?slaveOk=true", m_config.getConnectionUrl(m_primary, null, 1, false));
        assertEquals("mongodb://primary:1,one:1,two:1/?slaveOk=true", m_config.getConnectionUrl(m_primary, m_secondary, 1, false));
    }
}
