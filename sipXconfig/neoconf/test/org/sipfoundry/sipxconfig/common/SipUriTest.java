/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.common;

import java.util.Map;
import java.util.TreeMap;

import junit.framework.TestCase;

import org.apache.commons.lang.StringUtils;

public class SipUriTest extends TestCase {
    public void testFormatFullUri() {
        String uri = SipUri
                .formatIgnoreDefaultPort("First Last", "username", "example.com", 5060);
        assertEquals("\"First Last\"<sip:username@example.com>", uri);

        String uri2 = SipUri.formatIgnoreDefaultPort("First Last", "username", "example.com",
                5070);
        assertEquals("\"First Last\"<sip:username@example.com:5070>", uri2);
    }

    public void testParsePort() {
        assertEquals(5060, SipUri.parsePort("5060", 5070));
        assertEquals(5070, SipUri.parsePort("5070", 5060));
        assertEquals(5080, SipUri.parsePort(StringUtils.EMPTY, 5080));
        assertEquals(5080, SipUri.parsePort(null, 5080));
    }

    public void testFormatUser() {
        User user = new User();
        user.setUserName("username");
        String uri = SipUri.format(user, "mycomp.com");

        assertEquals("sip:username@mycomp.com", uri);

        user.setLastName("Last");
        uri = SipUri.format(user, "mycomp.com");

        assertEquals("\"Last\"<sip:username@mycomp.com>", uri);

        user.setFirstName("First");
        uri = SipUri.format(user, "mycomp.com");

        assertEquals("\"First Last\"<sip:username@mycomp.com>", uri.toString());
    }

    public void testFormatDisplayNameUserNameDomainName() {
        String uri = SipUri.format("Adam Słodowy", "adam", "zróbtosam.com");
        assertEquals("\"Adam Słodowy\"<sip:adam@zróbtosam.com>", uri);
        uri = SipUri.format("", "adam", "zróbtosam.com");
        assertEquals("sip:adam@zróbtosam.com", uri);
        uri = SipUri.format(null, "adam", "zróbtosam.com");
        assertEquals("sip:adam@zróbtosam.com", uri);
    }

    public void testFormatNameDomainPort() {
        String uri = SipUri.format("name", "sipfoundry.org", 33);
        assertEquals("sip:name@sipfoundry.org:33", uri);
    }

    public void testFormatDomainPort() {
        String uri = SipUri.format("sipfoundry.org", 34);
        assertEquals("sip:sipfoundry.org:34", uri);
    }

    public void testFormatNameDomain() {
        String uri = SipUri.format("name", "sipfoundry.org", false);
        assertEquals("sip:name@sipfoundry.org", uri);
    }

    public void testFormatNameDomainQuote() {
        String uri = SipUri.format("name", "sipfoundry.org", true);
        assertEquals("<sip:name@sipfoundry.org>", uri);
    }

    public void testExtractUser() {
        assertNull(SipUri.extractUser("name"));
        assertEquals("name", SipUri.extractUser("sip:name@sipfoundry.org"));
        assertEquals("name", SipUri.extractUser("<sip:name@sipfoundry.org>"));
        assertEquals("name", SipUri.extractUser("   <sip:name@sipfoundry.org>"));
        assertEquals("name", SipUri.extractUser("name@sipfoundry.org"));
        assertEquals("name", SipUri.extractUser("name@sipfoundry@.org"));
    }

    public void testExtractUserFromFullUser() {
        assertNull(SipUri.extractUser("name"));
        assertEquals("name", SipUri.extractUser("\"Joe \"<sip:name@sipfoundry.org>"));
        assertEquals("name", SipUri.extractUser("\"Joe Macy\"<sip:name@sipfoundry.org>"));
    }

    public void testExtractFullUser() {
        assertNull(SipUri.extractFullUser("name"));
        assertEquals("name", SipUri.extractFullUser("sip:name@sipfoundry.org"));
        assertEquals("name", SipUri.extractFullUser("<sip:name@sipfoundry.org>"));
        assertEquals("name", SipUri.extractFullUser("   <sip:name@sipfoundry.org>"));
        assertEquals("name", SipUri.extractFullUser("name@sipfoundry.org"));
        assertEquals("name", SipUri.extractFullUser("name@sipfoundry@.org"));

        assertEquals("first last - name", SipUri
                .extractFullUser("\"first last\"<sip:name@sipfoundry.org>"));
        assertEquals("Alice Smith - 180", SipUri
                .extractFullUser("\"Alice Smith\" <sip:180@example.com> "));
        
        assertEquals("Douglas+Hubler - 201", SipUri
                .extractFullUser("\"Douglas+Hubler\"<sip:201@nuthatch.pingtel.com>;tag%3D53585A61-338ED896"));        
    }

    public void testMatches() {
        assertTrue(SipUri.matches("\"Joe Macy\"<sip:name@sipfoundry.org>"));
        assertTrue(SipUri.matches("<sip:name@sipfoundry.org>"));
        assertTrue(SipUri.matches("sip:name@sipfoundry.org"));
        assertTrue(SipUri.matches("name@sipfoundry.org"));
        assertFalse(SipUri.matches("sip:namesipfoundry.org"));
        assertFalse(SipUri.matches("1234"));
    }

    public void testUrlParams() {
        Map urlParams = new TreeMap();
        urlParams.put("a-key", "aa");
        urlParams.put("b-key", null);
        urlParams.put("c-key", "cc");
        assertEquals("<sip:name@domain.com;a-key=aa;b-key;c-key=cc>", SipUri.format("name",
                "domain.com", urlParams));
    }

    public void testStripSipPrefix() {
        assertEquals("name@sipfoundry.org", SipUri.stripSipPrefix("sip:name@sipfoundry.org"));        
        assertEquals("name@sipfoundry.org", SipUri.stripSipPrefix("name@sipfoundry.org"));        
        assertNull(SipUri.stripSipPrefix(null));        
    }
}
