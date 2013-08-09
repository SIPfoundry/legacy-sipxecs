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
package org.sipfoundry.sipxconfig.region;

import java.util.Collection;
import java.util.List;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.test.IntegrationTestCase;

public class RegionManagerTestIntegration extends IntegrationTestCase {
    private RegionManager m_regionManager;    

    @Override
    protected void onSetUpBeforeTransaction() throws Exception {
        super.onSetUpBeforeTransaction();
        clear();
    }
    
    public void testCrud() {
        Region r1 = new Region();
        
        // Create
        r1.setName("Bird Bath in Central Park");
        m_regionManager.saveRegion(r1);
        String count = "select count(*) from region where region_id = ?";
        assertEquals(1, db().queryForInt(count, r1.getId()));
        
        // Read
        List<Region> regions = m_regionManager.getRegions();
        assertEquals(1, regions.size());
        String r1Expected = "Region[m_name=Bird Bath in Central Park,ID]";
        assertEquals(r1Expected, dump(regions));

        // Find
        Region r2 = new Region();
        String r2Expected = "Region[m_name=Statue of Liberty,ID]";
        r2.setName("Statue of Liberty");
        m_regionManager.saveRegion(r2);
        String actual = dump(m_regionManager.getRegions());
        assertEquals(r1Expected + '|' + r2Expected, actual);
        
        // Update
        r1.setName("Nesting");
        m_regionManager.saveRegion(r1);
        actual = dump(m_regionManager.getRegions());
        assertEquals("Region[m_name=Nesting,ID]|" + r2Expected, actual);
        
        // Delete
        m_regionManager.deleteRegion(r1);
        assertEquals(0, db().queryForInt(count, r1.getId()));        
    }
    
    String dump(Collection<?> o) {
        return clearIds(StringUtils.join(o, '|'));
    }
    
    String clearIds(String in) {
        return in.replaceAll("m_id=\\d+", "ID");
    }

    public void setRegionManager(RegionManager regionManager) {
        m_regionManager = regionManager;
    }
}
