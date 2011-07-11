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
