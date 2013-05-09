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

        assertEquals("c0ffee000000", Servlet.extractMac("/C0FFEE000000", "/"));
        assertEquals("c0ffee000000", Servlet.extractMac("/longer-c0ffee000000", "/longer-"));

        assertEquals(null, Servlet.extractMac("/c0ffee00000g", "/"));
        assertEquals(null, Servlet.extractMac("/c0ffee0000", "/"));
        assertEquals(null, Servlet.extractMac("fun", "/"));
    }

    public void testDoProvisionPhone() {

        Servlet servlet = new Servlet();
        Servlet.m_config = new Configuration();

        assertFalse(servlet.doProvisionPhone(null, null));
    }

    public void testLookupPhoneModelFailure() {

        assertEquals(null, Servlet.lookupPhoneModel("nope"));
    }

    public void testIsPolycom() {
        assertTrue(Servlet.isPolycomConfigurationFilePath("/0004f238a125-sipx-phone.cfg"));
        assertTrue(Servlet.isPolycomConfigurationFilePath("/0004f238a125-sipx-sip.cfg"));
        assertTrue(Servlet.isPolycomConfigurationFilePath("/0004f238a125-sipx-device.cfg"));
        assertTrue(Servlet.isPolycomConfigurationFilePath("/0004f238a125-sipx-reg-advanced.cfg"));
        assertTrue(Servlet.isPolycomConfigurationFilePath("/0004f238a125-sipx-applications.cfg"));
        assertTrue(Servlet.isPolycomConfigurationFilePath("/0004f238a125-sipx-sip-interop.cfg"));
        assertTrue(Servlet.isPolycomConfigurationFilePath("/0004f238a125-sipx-sip-basic.cfg"));
        assertTrue(Servlet.isPolycomConfigurationFilePath("/0004f238a125-sipx-region.cfg"));
        assertTrue(Servlet.isPolycomConfigurationFilePath("/0004f238a125-sipx-video.cfg"));
        assertTrue(Servlet.isPolycomConfigurationFilePath("/0004f238a125-sipx-site.cfg"));
        assertTrue(Servlet.isPolycomConfigurationFilePath("/0004f238a125-sipx-features.cfg"));
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
        assertEquals(true, Servlet.extractPolycomModelAndVersion(phone, "FileTransport PolycomSoundPointIP-SPIP_600-UA/3.1.3.0439"));
        assertNotNull(phone.model);
        assertEquals("polycom600", phone.model.sipxconfig_id);
        assertEquals("3.1.3.0439", phone.version);

        // Success
        phone = new DetectedPhone();
        assertEquals(true, Servlet.extractPolycomModelAndVersion(phone, "FileTransport PolycomSoundPointIP-SPIP_601-UA/3.1.3.0439"));
        assertNotNull(phone.model);
        assertEquals("polycom601", phone.model.sipxconfig_id);
        assertEquals("3.1.3.0439", phone.version);
        
        // Success
        phone = new DetectedPhone();
        assertEquals(true, Servlet.extractPolycomModelAndVersion(phone, "FileTransport PolycomSoundPointIP-VVX_500-UA/4.0.3.0439"));
        assertNotNull(phone.model);
        assertEquals("polycomVVX500", phone.model.sipxconfig_id);
        assertEquals("4.0.3.0439", phone.version);
        // TODO - test case that includes Serial Number string in th UA header.
    }

    public void testFormatPolycomVersion() {
        assertEquals("3.1.X",Servlet.formatPolycomVersion("3.1.3.0438"));
        assertEquals("3.2.X",Servlet.formatPolycomVersion("3.2.3.0438"));
        assertEquals("4.0.X",Servlet.formatPolycomVersion("4.0.3.0438"));
    }
    
    public void testExtractMacFromConfigurationFilePath() {
        assertEquals("c0ffee000016", Servlet.extractMac("/c0ffee000016-sipx-sip.cfg", Servlet.POLYCOM_PATH_PREFIX));
        assertEquals("002162ff374b", Servlet.extractMac("/Nortel/config/SIP002162FF374B.xml", Servlet.NORTEL_IP_12X0_PATH_PREFIX));

        // XX-8437 Support auto-provisioning Avaya IP 1200 Remote Worker phones
        assertEquals("002162ffb0ff", Servlet.extractMac("/phone/profile/tftproot/Nortel/config/SIP002162FFB0FF.xml",
            Servlet.NORTEL_IP_12X0_PATH_PREFIX));
    }

    public void testIsNortelIp12x0ConfigurationFilePath() {
        assertFalse(Servlet.isNortelIp12x0ConfigurationFilePath(""));
        assertFalse(Servlet.isNortelIp12x0ConfigurationFilePath("not a good path"));
        assertTrue(Servlet.isNortelIp12x0ConfigurationFilePath("/Nortel/config/SIP002162FFB0FF.xml"));

        // XX-8437 Support auto-provisioning Avaya IP 1200 Remote Worker phones
        String long_path = "/phone/profile/tftproot/Nortel/config/SIP002162FFB0FF.xml";
        assertTrue(Servlet.isNortelIp12x0ConfigurationFilePath(long_path));
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
        assertEquals("avaya-1210", phone.model.sipxconfig_id);
        assertEquals("SIP12x0.45.02.05.00", phone.version);

        // Success 1220
        phone = new DetectedPhone();
        assertEquals(true, Servlet.extractNortelIp12X0ModelAndVersion(phone, "Nortel IP Phone 1220 (SIP12x0.99.02.05.99)"));
        assertNotNull(phone.model);
        assertEquals("avaya-1220", phone.model.sipxconfig_id);
        assertEquals("SIP12x0.99.02.05.99", phone.version);

        // Success 1230
        phone = new DetectedPhone();
        assertEquals(true, Servlet.extractNortelIp12X0ModelAndVersion(phone, "Nortel IP Phone 1230 (SIP12x0.01.100.05.05)"));
        assertNotNull(phone.model);
        assertEquals("avaya-1230", phone.model.sipxconfig_id);
        assertEquals("SIP12x0.01.100.05.05", phone.version);
    }

}
