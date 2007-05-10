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
import org.sipfoundry.sipxconfig.device.ProfileContext;
import org.sipfoundry.sipxconfig.device.ProfileGenerator;
import org.sipfoundry.sipxconfig.device.VelocityProfileGenerator;
import org.sipfoundry.sipxconfig.phone.PhoneTestDriver;

public class SipConfigurationTest extends XMLTestCase {

    private PolycomPhone phone;

    private PhoneTestDriver tester;

    private ProfileGenerator m_pg;

    private MemoryProfileLocation m_location;

    protected void setUp() throws Exception {
        XMLUnit.setIgnoreWhitespace(true);
        PolycomModel model = new PolycomModel();
        model.setMaxLineCount(6);
        model.setModelId("polycom600");
        phone = new PolycomPhone();
        phone.setModel(model);
        tester = PhoneTestDriver.supplyTestData(phone);

        m_location = new MemoryProfileLocation();
        VelocityProfileGenerator pg = new VelocityProfileGenerator();
        pg.setVelocityEngine(TestHelper.getVelocityEngine());
        m_pg = pg;
    }

    public void testGenerateProfile16() throws Exception {
        phone.setDeviceVersion(PolycomModel.VER_1_6);
        assertProfileEquals("expected-sip.cfg.xml");
    }

    public void testGenerateProfile20() throws Exception {
        assertProfileEquals("expected-sip-2.0.cfg.xml");
    }

    private void assertProfileEquals(String expected) throws Exception {

        // settings selected at random, there are too many
        // to test all. select a few.
        phone.setSettingValue("log/sip/level.change.sip", "3");
        phone.setSettingValue("call/rejectBusyOnDnd", "0");
        phone.setSettingValue("voIpProt.SIP/local/port", "5061");
        phone.setSettingValue("call/rejectBusyOnDnd", "0");

        tester.getPrimaryLine().setSettingValue("call/serverMissedCall/enabled", "1");

        assertEquals("0", phone.getSettingValue("voIpProt.server.dhcp/available"));

        ProfileContext cfg = new SipConfiguration(phone);

        m_pg.generate(m_location, cfg, "profile");

        InputStream expectedPhoneStream = getClass().getResourceAsStream(expected);
        Reader expectedXml = new InputStreamReader(expectedPhoneStream);
        Reader generatedXml = m_location.getReader();

        // helpful debug
        // System.out.println(new String(out.toCharArray()));

        // also helpful
        // Writer w = new FileWriter("/tmp/delme");
        // IOUtils.write(out.toCharArray(), w);
        // w.close();

        Diff phoneDiff = new Diff(expectedXml, generatedXml);
        assertXMLEqual(phoneDiff, true);
        expectedPhoneStream.close();
    }
}
