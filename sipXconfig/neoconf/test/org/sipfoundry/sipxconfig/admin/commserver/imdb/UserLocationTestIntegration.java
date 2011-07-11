/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.admin.commserver.imdb;

import java.util.List;
import java.util.Map;

import org.sipfoundry.sipxconfig.IntegrationTestCase;

public class UserLocationTestIntegration extends IntegrationTestCase {

    private UserLocation m_location;

    public void testGenerateEmpty() throws Exception {
        loadDataSetXml("domain/DomainSeed.xml");
        List<Map<String, String>> items = ((DataSetGenerator) m_location).generate();
        assertEquals(0, items.size());
    }

    public void testGenerate() throws Exception {
        loadDataSetXml("domain/DomainSeed.xml");
        loadDataSet("admin/commserver/imdb/UserLocationSeed.xml");
        List<Map<String, String>> items = ((DataSetGenerator) m_location).generate();
        assertEquals(6, items.size());
        Map<String, String> item1 = items.get(0);
        assertEquals("sip:user1@example.org", item1.get("identity"));
        assertEquals("branch1", item1.get("location"));
        Map<String, String> item2 = items.get(1);
        assertEquals("sip:user2@example.org", item2.get("identity"));
        assertEquals("branch1", item2.get("location"));
        Map<String, String> item3 = items.get(2);
        assertEquals("sip:user3@example.org", item3.get("identity"));
        assertEquals("branch2", item3.get("location"));
        Map<String, String> item4 = items.get(3);
        assertEquals("sip:user4@example.org", item4.get("identity"));
        assertEquals("branch2", item4.get("location"));
        Map<String, String> item5 = items.get(4);
        assertEquals("sip:user5@example.org", item5.get("identity"));
        assertEquals("branch2", item5.get("location"));
        Map<String, String> item6 = items.get(5);
        assertEquals("sip:user6@example.org", item6.get("identity"));
        assertEquals("branch3", item6.get("location"));
    }

    public void setUserlocationDataSet(UserLocation location) {
        m_location = location;
    }
}
