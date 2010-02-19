/*
 *
 *
 * Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxprovision.auto;

import org.sipfoundry.sipxprovision.auto.Servlet.DetectedPhone;

import junit.framework.TestCase;

// TODO import org.easymock.EasyMock;
// TODO import org.easymock.IMocksControl;

/**
 * Tests for the Servlet class.
 *
 * @see Servlet
 *
 * @author Paul Mossman
 */
public class ServletTest extends TestCase {


    // TODO: Parse Polycom UAs with AND without Serial #s!!  (Version is the tricky part...)

    // TODO: Parse Polycom UA with "UA/ "  and "UA/", which may do interesting things to Version


    public void testExtractMac() {

        assertEquals("c0ffee000000", Servlet.extractMac("/c0ffee000000", "/"));
        assertEquals("c0ffee000000", Servlet.extractMac("/longer-c0ffee000000", "/longer-"));

        assertEquals(null, Servlet.extractMac("/c0ffee00000g", "/"));
        assertEquals(null, Servlet.extractMac("/c0ffee0000", "/"));
        assertEquals(null, Servlet.extractMac("fun", "/"));
    }

    public void testDoProvisionPhone() {

        Servlet servlet = new Servlet();
        Servlet.m_config = new Configuration();

        assertFalse(servlet.doProvisionPhone(null));
    }

    public void testLookupPhoneModelFailure() {

        assertEquals(null, Servlet.lookupPhoneModel("nope"));
    }

    public void testExtractPolycomModelAndVersion() {

        DetectedPhone phone = new DetectedPhone();

        // Don't crash.
        assertEquals(false, Servlet.extractPolycomModelAndVersion(null, "FileTransport PolycomSoundStationIP-SSIP_6000-UA/3.2.0.0157"));
        assertEquals(false, Servlet.extractPolycomModelAndVersion(phone, null));
        assertEquals(false, Servlet.extractPolycomModelAndVersion(phone, ""));

        // Success
        phone = new DetectedPhone();
        assertEquals(true, Servlet.extractPolycomModelAndVersion(phone, "FileTransport PolycomSoundStationIP-SSIP_6000-UA/3.2.0.0157"));
        assertNotNull(phone.model);
        assertEquals("polycom6000", phone.model.sipxconfig_id);
        assertEquals("3.2.0.0157", phone.version);

        // Success
        phone = new DetectedPhone();
        assertEquals(true, Servlet.extractPolycomModelAndVersion(phone, "FileTransport PolycomSoundPointIP-SPIP_601-UA/3.1.3.0439"));
        assertNotNull(phone.model);
        assertEquals("polycom600", phone.model.sipxconfig_id);
        assertEquals("3.1.3.0439", phone.version);

        // TODO - test case that includes Serial Number string in th UA header.
    }

    public void testExtractNortelIp12X0ModelAndVersion() {

        DetectedPhone phone = new DetectedPhone();

        // Don't crash.
        assertEquals(false, Servlet.extractNortelIp12X0ModelAndVersion(null, "Nortel IP Phone 1230 (SIP12x0.01.02.05.00)"));
        assertEquals(false, Servlet.extractNortelIp12X0ModelAndVersion(phone, null));
        assertEquals(false, Servlet.extractNortelIp12X0ModelAndVersion(phone, ""));
        assertEquals(false, Servlet.extractNortelIp12X0ModelAndVersion(phone, "Nortel IP Phone 1290 (SIP12x0.01.02.05.00)"));
        assertEquals(false, Servlet.extractNortelIp12X0ModelAndVersion(phone, "Nortel IP Phone 1230"));
        assertEquals(false, Servlet.extractNortelIp12X0ModelAndVersion(phone, "Nortel IP Phone 1230 (SIP12x0.01.02.05.00"));
        assertEquals(false, Servlet.extractNortelIp12X0ModelAndVersion(phone, "Nortel IP Phone 1230 (SAP12x0.01.02.05.00)"));
        assertEquals(false, Servlet.extractNortelIp12X0ModelAndVersion(phone, "Nortel IP Phone 1230 (12x0.01.02.05.00)"));
        assertEquals(false, Servlet.extractNortelIp12X0ModelAndVersion(phone, "Nortel IP Phone 1230 (SIP12x0.99.02.05.00a)"));
        assertEquals(false, Servlet.extractNortelIp12X0ModelAndVersion(phone, "Nortel IP Phone 1230 (SIP12x9.01.02.05.00)"));

        // Success 1210
        phone = new DetectedPhone();
        assertEquals(true, Servlet.extractNortelIp12X0ModelAndVersion(phone, "Nortel IP Phone 1210 (SIP12x0.45.02.05.00)"));
        assertNotNull(phone.model);
        assertEquals("nortel-1210", phone.model.sipxconfig_id);
        assertEquals("SIP12x0.45.02.05.00", phone.version);

        // Success 1220
        phone = new DetectedPhone();
        assertEquals(true, Servlet.extractNortelIp12X0ModelAndVersion(phone, "Nortel IP Phone 1220 (SIP12x0.99.02.05.99)"));
        assertNotNull(phone.model);
        assertEquals("nortel-1220", phone.model.sipxconfig_id);
        assertEquals("SIP12x0.99.02.05.99", phone.version);

        // Success 1230
        phone = new DetectedPhone();
        assertEquals(true, Servlet.extractNortelIp12X0ModelAndVersion(phone, "Nortel IP Phone 1230 (SIP12x0.01.100.05.05)"));
        assertNotNull(phone.model);
        assertEquals("nortel-1230", phone.model.sipxconfig_id);
        assertEquals("SIP12x0.01.100.05.05", phone.version);
    }

}
