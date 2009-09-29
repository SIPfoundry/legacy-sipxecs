/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.commserver.imdb;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.admin.forwarding.AliasMapping;

public class AliasesTest extends TestCase {
    private final static String[][] DATA = {
        {
            "301@example.org", "\"John Doe\"<sip:john.doe@example.org>"
        }, {
            "302@example.org", "\"Jane Doe\"<sip:jane.doe@example.org>;q=0.5"
        }, {
            "302@example.org", "\"Betty Boop\"<sip:betty.boop@example.org>;q=0.8"
        }, {
            "302@example.org", "\"Bill Boop\"<sip:bill.boop@example.org>;q=0.8"
        }, {
            "303@example.org", "\"John Doe\"<sip:john.doe@example.org>"
        }
    };

    private Aliases m_aliases;

    protected void setUp() throws Exception {
        m_aliases = new Aliases();
    }

    public void testAddAliases() throws Exception {
        List<AliasMapping> aliases = new ArrayList<AliasMapping>();
        for (int i = 0; i < DATA.length; i++) {
            String[] aliasRow = DATA[i];
            AliasMapping mapping = new AliasMapping();
            mapping.setIdentity(aliasRow[0]);
            mapping.setContact(aliasRow[1]);
            aliases.add(mapping);
        }

        List<Map<String, String>> result = new ArrayList<Map<String, String>>();
        m_aliases.addAliases(result, aliases);

        assertEquals(DATA.length, result.size());
        for (int i = 0; i < DATA.length; i++) {
            String[] aliasRow = DATA[i];
            assertEquals(aliasRow[0], result.get(i).get("identity"));
            assertEquals(aliasRow[1], result.get(i).get("contact"));
        }
    }
}
