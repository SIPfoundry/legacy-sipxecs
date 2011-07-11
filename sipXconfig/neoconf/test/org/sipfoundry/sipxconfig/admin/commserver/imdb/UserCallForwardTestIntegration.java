/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
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
