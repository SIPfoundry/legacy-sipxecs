/**
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

import java.io.IOException;
import java.io.StringWriter;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;

import org.apache.commons.io.IOUtils;
import org.junit.Before;
import org.junit.Test;
import org.sipfoundry.sipxconfig.commserver.Location;

public class MongoConfigTest {
    private MongoConfig m_config;
    private Integer regionOne = 1;
    private Location m_s1;
    private Location m_s2;
    private List<Location> m_single;
    private List<Location> m_multi;
    
    @Before
    public void setUp() {
        m_config = new MongoConfig();
        m_s1 = new Location("one");
        m_s1.setRegionId(regionOne);
        m_s2 = new Location("two");
        m_multi = Arrays.asList(m_s1, m_s2);
        m_single = Collections.singletonList(m_s1);
    }
    
    @Test
    public void getConnectionString() {
        assertEquals("sipxecs/one:1", m_config.getConnectionString(m_single, "sipxecs", 1));
        assertEquals("sipxecs/one:1,two:1", m_config.getConnectionString(m_multi, "sipxecs", 1));
    }

    @Test
    public void getConnectionUrl() {
        assertEquals("mongodb://one:1/?readPreference=nearest&readPreferenceTags=shardId:99;readPreferenceTags=clusterId:1;readPreferenceTags=", m_config.getConnectionUrl(m_single, 99, true, 1));
        assertEquals("mongodb://one:1,two:1/?readPreference=nearest", m_config.getConnectionUrl(m_multi, 66, false, 1));
    }

    @Test
    public void getServerList() throws IOException {
        Location s3 = new Location("three");
        Location s4 = new Location("four");
        Arrays.asList(s3, s4);
        StringWriter actual = new StringWriter();
        String expected = IOUtils.toString(getClass().getResourceAsStream("server-list.expected.json"));
        m_config.modelFile(actual, m_multi, Arrays.asList(s3, s4), "sipxecs", false, 27017, 27018);
        assertEquals(expected, actual.toString());
    }
}
