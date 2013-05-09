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

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;

import java.io.IOException;
import java.util.Collection;

import org.apache.commons.io.IOUtils;
import org.junit.Before;
import org.junit.Test;

public class MongoMetaTest {
    private MongoMeta m_meta;
    
    @Before
    public void setUp() {
        m_meta = new MongoMeta();
    }
    
    @Test
    public void getNodesByServer() throws IOException {
        String token = IOUtils.toString(getClass().getResourceAsStream("three-node-healthy.status.json"));
        m_meta.setStatusToken(token);
        Collection<MongoNode> nodes = m_meta.getNodes();
        assertEquals(3, nodes.size());
        assertNotNull(m_meta.getNode("swift.hubler.us:27017"));
        assertEquals("swift.hubler.us:27017", m_meta.getPrimary().getHostPort());
    }

    @Test
    public void getAnalyis() throws IOException {
        String analToken = IOUtils.toString(getClass().getResourceAsStream("three-node-missing-arbiter-and-database.analysis.json"));
        m_meta.setAnalysisToken(analToken);
        Collection<String> actual = m_meta.getRequiredActions("swift.hubler.us:27017");
        assertEquals("[ \"ADD swift.hubler.us:27018\" , \"ADD swift.hubler.us:27019\"]", actual.toString());
    }

    @Test
    public void getStatus() throws IOException {
        String token = IOUtils.toString(getClass().getResourceAsStream("two-node-healthy.status.json"));
        m_meta.setStatusToken(token);
        assertNotNull(m_meta.getNodes());
    }
}
