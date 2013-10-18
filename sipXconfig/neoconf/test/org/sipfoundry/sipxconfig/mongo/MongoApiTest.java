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

import static org.easymock.EasyMock.createNiceMock;
import static org.easymock.EasyMock.replay;

import java.io.IOException;
import java.util.Collections;
import java.util.Map;

import org.apache.commons.io.IOUtils;
import org.junit.Before;
import org.junit.Test;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.domain.Domain;
import org.sipfoundry.sipxconfig.domain.DomainManagerImpl;
import org.sipfoundry.sipxconfig.region.Region;
import org.sipfoundry.sipxconfig.region.RegionManager;
import org.sipfoundry.sipxconfig.test.TestHelper;

public class MongoApiTest {
    private MongoApi m_api;
    private MongoManagerImpl m_mgr;
    
    @Before
    public void setUp() {
        MongoReplSetManager rsMgr = createNiceMock(MongoReplSetManager.class);
        replay(rsMgr);
        RegionManager regionMgr = createNiceMock(RegionManager.class);
        replay(regionMgr);
        Domain d = new Domain("hubler.us");
        d.setNetworkName("hubler.us");
        new DomainManagerImpl().setTestDomain(d);
        m_api = new MongoApi();
        m_mgr = new MongoManagerImpl();
        m_api.setMongoManager(m_mgr);
        m_mgr.setGlobalManager(rsMgr);
        m_api.setRegionManager(regionMgr);
    }
    
    @Test
    public void healthy() throws IOException {
        MongoMeta meta = testData("three-node-healthy");
        Region r1 = new Region("r1");
        r1.setUniqueId(1);
        Location l1 = new Location("swift.hubler.us");
        l1.setRegionId(r1.getId());
        MongoNode node = meta.getNode("swift.hubler.us:27017");
        Map<String, Object> actual = m_api.metaNode(node, meta, l1, false, Collections.singletonList(r1));
        String expected = IOUtils.toString(getClass().getResourceAsStream("three-node-healthy.expected.json"));
        TestHelper.assertEquals(expected, actual);        
    }

    @Test
    public void missing() throws IOException {
        MongoMeta meta = testData("three-node-missing-arbiter-and-database");
        // region defined but we'll test location not using a region
        Region r1 = new Region("r1");
        r1.setUniqueId(1);
        Location l1 = new Location("swift.hubler.us");
        MongoNode node = meta.getNode("swift.hubler.us:27018");
        Map<String, Object> actual = m_api.metaNode(node, meta, l1, true, Collections.singletonList(r1));
        String expected = IOUtils.toString(getClass().getResourceAsStream("three-node-missing-arbiter-and-database.expected.json"));
        TestHelper.assertEquals(expected, actual);        
    }

    MongoMeta testData(String id) throws IOException {
        MongoMeta meta = new MongoMeta();
        String status = IOUtils.toString(getClass().getResourceAsStream(id + ".status.json"));
        meta.setStatusToken(status);
        String analysis = IOUtils.toString(getClass().getResourceAsStream(id + ".analysis.json")); 
        meta.setAnalysisToken(analysis);
        return meta;
    }
}
