/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.phone.polycom;

import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.Reader;

import org.custommonkey.xmlunit.Diff;
import org.custommonkey.xmlunit.XMLTestCase;
import org.custommonkey.xmlunit.XMLUnit;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.device.MemoryProfileLocation;
import org.sipfoundry.sipxconfig.device.ProfileGenerator;
import org.sipfoundry.sipxconfig.device.VelocityProfileGenerator;
import org.sipfoundry.sipxconfig.phone.PhoneTestDriver;

/**
 * Tests file [MAC_ADDRESS.d]/phone.cfg
 */
public class PhoneConfigurationTest extends XMLTestCase {

    private PolycomPhone phone;
    private ProfileGenerator m_pg;
    private MemoryProfileLocation m_location;

    protected void setUp() throws Exception {
        XMLUnit.setIgnoreWhitespace(true);
        phone = new PolycomPhone();
        phone.getModel().setMaxLineCount(6);
        PhoneTestDriver.supplyTestData(phone);

        m_location = new MemoryProfileLocation();
        VelocityProfileGenerator pg = new VelocityProfileGenerator();
        pg.setVelocityEngine(TestHelper.getVelocityEngine());
        pg.setProfileLocation(m_location);
        m_pg = pg;
    }

    public void testGenerateProfileVersion16() throws Exception {
        phone.setDeviceVersion(PolycomModel.VER_1_6);
        assertExpectedProfile("expected-phone.cfg.xml");
    }

    public void testGenerateProfileVersion20() throws Exception {
        assertExpectedProfile("expected-phone-2.0.cfg.xml");
    }

    private void assertExpectedProfile(String expected) throws Exception {
        PhoneConfiguration cfg = new PhoneConfiguration(phone);
        m_pg.generate(cfg, "profile");

        InputStream expectedPhoneStream = getClass().getResourceAsStream(expected);
        Reader expectedXml = new InputStreamReader(expectedPhoneStream);
        Reader generatedXml = m_location.getReader();

        Diff phoneDiff = new Diff(expectedXml, generatedXml);
        assertXMLEqual(phoneDiff, true);
        expectedPhoneStream.close();
    }
}
