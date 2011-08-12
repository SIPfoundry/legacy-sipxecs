/**
 *
 *
 * Copyright (c) 2010 / 2011 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.admin.commserver.imdb;

import java.util.List;
import java.util.Map;

import org.sipfoundry.sipxconfig.IntegrationTestCase;

public class UserCallForwardTestIntegration extends IntegrationTestCase {

    private UserForward m_forward;

    public void testGenerateEmpty() throws Exception {
        loadDataSetXml("domain/DomainSeed.xml");
        List<Map<String, String>> items = ((DataSetGenerator) m_forward).generate();
        assertEquals(0, items.size());
    }

    public void testGenerate() throws Exception {
        loadDataSetXml("domain/DomainSeed.xml");
        loadDataSet("admin/commserver/imdb/UserForwardSeed.xml");
        List<Map<String, String>> items = ((DataSetGenerator) m_forward).generate();
        // 3 users, first one with cfwd time 8, 2nd with the default one (20) and third with 192
        // inherited from user group
        assertEquals(3, items.size());
        Map<String, String> item1 = items.get(0);
        assertEquals("alpha@example.org", item1.get("identity"));
        assertEquals("8", item1.get("cfwdtime"));
        Map<String, String> item2 = items.get(1);
        assertEquals("alpha1@example.org", item2.get("identity"));
        assertEquals("20", item2.get("cfwdtime"));
        Map<String, String> item3 = items.get(2);
        assertEquals("alpha3@example.org", item3.get("identity"));
        assertEquals("192", item3.get("cfwdtime"));
    }

    public void setUserforwardDataSet(UserForward forward) {
        m_forward = forward;
    }
}
