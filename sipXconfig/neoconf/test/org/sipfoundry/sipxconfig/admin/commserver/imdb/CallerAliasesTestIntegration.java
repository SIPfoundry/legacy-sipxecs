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

public class CallerAliasesTestIntegration extends IntegrationTestCase {

    private CallerAliases m_aliases;

    public void testGenerate() throws Exception {
        loadDataSetXml("domain/DomainSeed.xml");
        loadDataSet("gateway/seed_ds_gateway.db.xml");
        loadDataSet("common/UserSearchSeed.xml");
        List<Map<String, String>> callerAliases = ((DataSetGenerator) m_aliases).generate();
        assertEquals(11, callerAliases.size());
        Map<String, String> gw1Alias = callerAliases.get(0);
        assertNull(gw1Alias.get("identity"));
        assertEquals("192.168.0.119;sipxecs-lineid=1002", gw1Alias.get("domain"));
        assertEquals("sip:545454@example.org", gw1Alias.get("alias"));

        Map<String, String> user = callerAliases.get(1);
        assertEquals("userseed1@example.org", user.get("identity"));
        assertEquals("192.168.0.119;sipxecs-lineid=1002", user.get("domain"));
        assertEquals("sip:41@example.org", user.get("alias"));

        Map<String, String> user3 = callerAliases.get(3);
        assertEquals("userseed3@example.org", user3.get("identity"));
        assertEquals("192.168.0.119;sipxecs-lineid=1002", user3.get("domain"));
        assertEquals("sip:41@example.org", user3.get("alias"));
    }

    public void setCalleraliasDataSet(CallerAliases callerAliases) {
        m_aliases = callerAliases;
    }

}
