package org.sipfoundry.sipxconfig.admin;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;

import org.junit.Test;

@SuppressWarnings("static-method")
public class AdminSettingsTest {
    private static final String IP4_ADDR1 = "127.0.0.1";
    private static final String IP4_ADDR2 = "10.11.12.13";
    private static final String IP6_ADDR1 = "::1";
    private static final String IP6_ADDR2 = "2607:f0d0:1002:0051:0000:0000:0000:0004";
    private static final String DOMAIN1 = "someDomain";
    private static final String DOMAIN2 = "some-other-domain";
    private static final String FQDN1 = DOMAIN1 + ".com";
    private static final String FQDN2 = "www." + DOMAIN2 + ".org";

    @Test
    public void testCorsIp4() {
        assertTrue(propertyOk(IP4_ADDR1));
        assertTrue(propertyOk(IP4_ADDR2));
    }

    @Test
    public void testCorsIp4WithWhitespace() {
        assertTrue(propertyOk("\t" + IP4_ADDR1 + " \n"));
    }

    @Test
    public void testCorsIp4WhitespaceTrim() {
        assertEquals(IP4_ADDR1, propertySet("\t " + IP4_ADDR1 + "  \n"));
    }

    @Test
    public void testCorsIp6() {
        assertFalse(propertyOk(IP6_ADDR1));
        assertFalse(propertyOk(IP6_ADDR2));
    }

    @Test
    public void testCorsSimpleDomain() {
        assertTrue(propertyOk(DOMAIN1));
        assertTrue(propertyOk(DOMAIN2));
    }

    @Test
    public void testCorsSimpleDomainWithWhitespace() {
        assertTrue(propertyOk("\n\n " + DOMAIN1 + "\t\r"));
    }

    @Test
    public void testCorsSimpleDomainWhitespaceTrim() {
        assertEquals(DOMAIN1, propertySet("\n\n " + DOMAIN1 + "\t\r"));
    }

    @Test
    public void testCorsFqdn() {
        assertTrue(propertyOk(FQDN1));
        assertTrue(propertyOk(FQDN2));
    }

    @Test
    public void testCorsDomainList() {
        assertTrue(propertyOk(FQDN1 + "," + FQDN2));
    }

    @Test
    public void testCorsDomainListWithEverything() {
        assertTrue(propertyOk("\t" + IP4_ADDR2 + " , \r" + DOMAIN1 + " \n, " + FQDN2 + "  "));
    }

    @Test
    public void testCorsDomainListWithEverythingTrim() {
        assertEquals(IP4_ADDR2 + "," + DOMAIN1 + "," + FQDN2, propertySet("\t" + IP4_ADDR2 + " , \r" + DOMAIN1
            + " \n, " + FQDN2 + "  "));
    }

    @Test
    public void testCorsDomainListInvalidChar() {
        assertFalse(propertyOk(FQDN1 + "," + FQDN2 + " +"));
    }

    @Test
    public void testCorsDomainNull() {
        assertEquals("", propertySet(null));
    }

    private static boolean propertyOk(String corsDomainList) {
        boolean exceptionThrown = false;

        try {
            propertySet(corsDomainList);
        } catch (IllegalArgumentException e) {
            exceptionThrown = true;
        }

        return !exceptionThrown;
    }

    private static String propertySet(String corsDomainList) {
        return AdminSettings.validateDomainList(corsDomainList);
    }
}
