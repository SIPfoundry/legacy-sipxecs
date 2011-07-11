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

public class UserStaticTestIntegration extends IntegrationTestCase {
    private UserStatic m_static;

    public void testGenerateEmpty() throws Exception {
        loadDataSetXml("domain/DomainSeed.xml");
        List<Map<String, String>> items = ((DataSetGenerator) m_static).generate();
        assertEquals(0, items.size());
    }

    public void testGenerate() throws Exception {
        loadDataSetXml("domain/DomainSeed.xml");
        loadDataSet("admin/commserver/imdb/UserStaticSeed.xml");
        List<Map<String, String>> items = ((DataSetGenerator) m_static).generate();
        assertEquals(2, items.size());
        Map<String, String> item = items.get(0);
        assertEquals("sip:IVR@example.org", item.get("from_uri"));
        assertEquals("sip:test-mwi@example.org", item.get("contact"));
        assertEquals("alpha@example.org", item.get("identity"));
        assertEquals("static-mwi-alpha@example.org", item.get("callid"));
        assertEquals("message-summary", item.get("event"));
        assertEquals("sip:alpha@example.org", item.get("to_uri"));
    }

    public void setUserstaticDataSet(UserStatic userStatic) {
        m_static = userStatic;
    }
}
