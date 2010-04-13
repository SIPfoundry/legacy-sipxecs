/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.api;

import java.util.Arrays;
import java.util.Set;
import java.util.TreeSet;

import junit.framework.TestCase;

public class SystemInfoBuilderTest extends TestCase {
    private SystemInfoBuilder m_builder;
    private org.sipfoundry.sipxconfig.domain.Domain m_myDomain;
    private Domain m_apiDomain;

    private static final String ONEDOMAIN = "onedomain";
    private static final String TWODOMAIN = "twodomain";
    private static final String DOMAINNAME = "domainname";
    private static final String ANOTHERDOMAINNAME = "newdomainname";
    private static final String REALM = "thisisrealm";

    protected void setUp() {
        m_builder = new SystemInfoBuilder();
        m_myDomain = new org.sipfoundry.sipxconfig.domain.Domain();
        m_apiDomain = new Domain();
    }

    public void testFromApi() {
        m_apiDomain.setAliases(new String[] {
            ONEDOMAIN, TWODOMAIN
        });
        m_apiDomain.setName(DOMAINNAME);
        m_apiDomain.setRealm(REALM);
        ApiBeanUtil.toMyObject(m_builder, m_myDomain, m_apiDomain);

        assertEquals(DOMAINNAME, m_myDomain.getName());
        assertEquals(true, m_myDomain.hasAliases());

        Set<String> aliases = m_myDomain.getAliases();
        assertNotNull(aliases);
        assertEquals(2, aliases.size());
        String[] array = aliases.toArray(new String[2]);
        assertNotNull(array);

        // NOTE: order is not important
        Arrays.sort(array);
        assertEquals(ONEDOMAIN, array[0]);
        assertEquals(TWODOMAIN, array[1]);
    }

    public void testToApi() {
        m_apiDomain.setRealm("");
        String[] aliases = new String[] {
            ONEDOMAIN, TWODOMAIN
        };
        m_myDomain.setAliases(new TreeSet<String>(Arrays.asList(aliases)));
        m_myDomain.setName(ANOTHERDOMAINNAME);
        ApiBeanUtil.toApiObject(m_builder, m_apiDomain, m_myDomain);
        assertEquals(ANOTHERDOMAINNAME, m_apiDomain.getName());
        assertEquals(ONEDOMAIN, m_apiDomain.getAliases(0));
        assertEquals(TWODOMAIN, m_apiDomain.getAliases(1));
        assertEquals("", m_apiDomain.getRealm());
    }
}
