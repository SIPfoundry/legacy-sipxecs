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
        String uri = SipUri.formatIgnoreDefaultPort("First Last", "username", "example.com", 5060);
        assertEquals("\"First Last\"<sip:username@example.com>", uri);

        String uri2 = SipUri.formatIgnoreDefaultPort("First Last", "username", "example.com", 5070);
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
        String uri = SipUri.format("Adam S\u00f3odowy", "adam", "zrobtosam.com");
        assertEquals("\"Adam S\u00f3odowy\"<sip:adam@zrobtosam.com>", uri);
        uri = SipUri.format("", "adam", "zrobtosam.com");
        assertEquals("sip:adam@zrobtosam.com", uri);
        uri = SipUri.format(null, "adam", "zrobtosam.com");
        assertEquals("sip:adam@zrobtosam.com", uri);
    }

    public void testFormatNameDomainPort() {
        String uri = SipUri.format("name", "sipfoundry.org", 33);
        assertEquals("sip:name@sipfoundry.org:33", uri);

        uri = SipUri.format("name", "sipfoundry.net", 0);
        assertEquals("sip:name@sipfoundry.net", uri);
    }

    public void testFormatDomainPort() {
        String uri = SipUri.format("sipfoundry.org", 34);
        assertEquals("sip:sipfoundry.org:34", uri);

        uri = SipUri.format("sipfoundry.com", 0);
        assertEquals("sip:sipfoundry.com", uri);
    }

    public void testFormatNameDomain() {
        String uri = SipUri.format("name", "sipfoundry.org", false);
        assertEquals("sip:name@sipfoundry.org", uri);
    }

    public void testFormatNameDomainQuote() {
        String uri = SipUri.format("name", "sipfoundry.org", true);
        assertEquals("<sip:name@sipfoundry.org>", uri);
    }

    public void testFormatNameDomainPortNoQuote() {
        String uri = SipUri.format("name", "sipfoundry.org", 5070, false);
        assertEquals("sip:name@sipfoundry.org:5070", uri);

        uri = SipUri.format("name", "sipfoundry.net", 0, false);
        assertEquals("sip:name@sipfoundry.net", uri);
    }

    public void testFormatNameDomainPortQuote() {
        String uri = SipUri.format("name", "sipfoundry.org", 5050, true);
        assertEquals("<sip:name@sipfoundry.org:5050>", uri);

        uri = SipUri.format("name", "sipfoundry.gov", 0, true);
        assertEquals("<sip:name@sipfoundry.gov>", uri);
    }

    public void testExtractUser() {
        assertNull(SipUri.extractUser("name"));
        assertEquals("name", SipUri.extractUser("sip:name@sipfoundry.org"));
        assertEquals("name", SipUri.extractUser("<sip:name@sipfoundry.org>"));
        assertEquals("name", SipUri.extractUser("   <sip:name@sipfoundry.org>"));
        assertEquals("name", SipUri.extractUser("name@sipfoundry.org"));
        assertEquals("name", SipUri.extractUser("name@sipfoundry@.org"));
        assertEquals("O", SipUri.extractUser("A <sip:O@us.calivia.com>"));
        assertEquals("name", SipUri.extractUser("\"*123\"<sip:name@sipfoundry@.org>"));
        assertEquals("name", SipUri.extractUser("\"*123 then some more junk and more * characters\"<sip:name@sipfoundry@.org>"));
        assertEquals("name", SipUri.extractUser("\"*123 then some more junk and more * characters\"<sip:name;phone_context=sipfoundry.org>"));
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
        assertEquals("name", SipUri.extractFullUser("<sip:name;phone_context=sipfoundry.org>"));

        String fullUser = SipUri.extractFullUser("\"first last\"<sip:name@sipfoundry.org>");
        assertEquals("first last - name", fullUser);
        fullUser = SipUri.extractFullUser("\"Alice Smith\" <sip:180@example.com> ");
        assertEquals("Alice Smith - 180", fullUser);

        fullUser = SipUri
                .extractFullUser("\"Douglas+Hubler\"<sip:201@nuthatch.pingtel.com>;tag%3D53585A61-338ED896");
        assertEquals("Douglas+Hubler - 201", fullUser);
        fullUser = SipUri
                .extractFullUser("\"Douglas+Hubler\"<sip:201;phone_context=pingtel.com>;tag%3D53585A61-338ED896");
        assertEquals("Douglas+Hubler - 201", fullUser);

        assertEquals("O", SipUri.extractFullUser("O <sip:O@us.calivia.com>"));
        assertEquals("Abc - 1234", SipUri.extractFullUser("Abc <sip:1234@us.calivia.com>"));
    }

    public void testAddressSpec() {
        assertNull(SipUri.extractAddressSpec(""));
        assertNull(SipUri.extractAddressSpec(null));
        assertNull(SipUri.extractAddressSpec("name"));

        assertEquals("sip:name@sipfoundry.org", SipUri.extractAddressSpec("sip:name@sipfoundry.org"));
        assertEquals("sip:name@sipfoundry.org", SipUri.extractAddressSpec("   <sip:name@sipfoundry.org>"));
        assertEquals("sip:name@sipfoundry.org", SipUri.extractAddressSpec("<sip:name@sipfoundry.org>"));

        String spec = SipUri.extractAddressSpec("\"first last\"<sip:name@sipfoundry.org>");
        assertEquals("sip:name@sipfoundry.org", spec);

        spec = SipUri.extractAddressSpec("\"Douglas+Hubler\"<sip:201@nuthatch.pingtel.com>;tag%3D53585A61-338ED896");
        assertEquals("sip:201@nuthatch.pingtel.com", spec);
    }

    public void testMatches() {
        assertTrue(SipUri.matches("\"Joe Macy\"<sip:name@sipfoundry.org>"));
        assertTrue(SipUri.matches("<sip:name@sipfoundry.org>"));
        assertTrue(SipUri.matches("sip:name@sipfoundry.org"));
        assertTrue(SipUri.matches("name@sipfoundry.org"));
        assertFalse(SipUri.matches("sip:namesipfoundry.org"));
        assertFalse(SipUri.matches("1234"));
        assertFalse(SipUri.matches(""));
        assertFalse(SipUri.matches(null));
    }

    public void testFix() {
        assertEquals("sip:name@sipfoundry.org", SipUri.fix("sip:name@sipfoundry.org", "example.org"));
        assertEquals("sip:name@sipfoundry.org", SipUri.fix("name@sipfoundry.org", "example.org"));
        assertEquals("sip:name@example.org", SipUri.fix("name", "example.org"));
        assertEquals("", SipUri.fix("", "example.org"));
        assertNull(SipUri.fix(null, "example.org"));
    }

    public void testFixWithDisplayName() {
        assertEquals("sip:123@example.org", SipUri.fixWithDisplayName("123", "", "", "example.org"));
        assertEquals("\"display name\"<sip:123@example.org>", SipUri.fixWithDisplayName("123", "display name", "",
                "example.org"));
        assertEquals("sip:123@example.org;key=value", SipUri.fixWithDisplayName("123", "", "key=value",
                "example.org"));
        assertEquals("\"display name\"<sip:123@example.org;key=value>", SipUri.fixWithDisplayName("123",
                "display name", "key=value", "example.org"));
        assertEquals("sip:name@sipfoundry.org", SipUri.fixWithDisplayName("name@sipfoundry.org", "", "",
                "example.org"));
        assertEquals("\"display name\"<sip:name@sipfoundry.org>", SipUri.fixWithDisplayName("name@sipfoundry.org",
                "display name", "", "example.org"));
        assertEquals("sip:name@sipfoundry.org;key=value", SipUri.fixWithDisplayName("name@sipfoundry.org", "",
                "key=value", "example.org"));
        assertEquals("\"display name\"<sip:name@sipfoundry.org;key=value>", SipUri.fixWithDisplayName(
                "name@sipfoundry.org", "display name", "key=value", "example.org"));
    }

    public void testUrlParams() {
        Map urlParams = new TreeMap();
        urlParams.put("a-key", "aa");
        urlParams.put("b-key", null);
        urlParams.put("c-key", "cc");
        assertEquals("<sip:name@domain.com;a-key=aa;b-key;c-key=cc>", SipUri.format("name", "domain.com", urlParams));
        assertEquals("<sip:name@domain.com:1234;a-key=aa;b-key;c-key=cc>", SipUri.format("name", "domain.com", 1234,
                urlParams));
    }

    public void testStripSipPrefix() {
        assertEquals("name@sipfoundry.org", SipUri.stripSipPrefix("sip:name@sipfoundry.org"));
        assertEquals("name@sipfoundry.org", SipUri.stripSipPrefix("name@sipfoundry.org"));
        assertNull(SipUri.stripSipPrefix(null));
    }
}
