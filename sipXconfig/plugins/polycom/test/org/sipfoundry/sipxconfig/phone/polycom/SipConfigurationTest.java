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
import java.util.HashSet;
import java.util.Set;

import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.device.MemoryProfileLocation;
import org.sipfoundry.sipxconfig.device.ProfileContext;
import org.sipfoundry.sipxconfig.device.ProfileGenerator;
import org.sipfoundry.sipxconfig.device.VelocityProfileGenerator;
import org.sipfoundry.sipxconfig.phone.PhoneTestDriver;

public class SipConfigurationTest extends PolycomXmlTestCase {

    private PolycomPhone phone;

    private PhoneTestDriver tester;

    private ProfileGenerator m_pg;

    private MemoryProfileLocation m_location;

    @Override
    protected void setUp() throws Exception {
        phone = new PolycomPhone();

        m_location = new MemoryProfileLocation();
        VelocityProfileGenerator pg = new VelocityProfileGenerator();
        pg.setVelocityEngine(TestHelper.getVelocityEngine());
        m_pg = pg;
    }

    public void testGenerateProfile20() throws Exception {
        PolycomModel model = new PolycomModel();
        model.setMaxLineCount(6);
        model.setModelId("polycom600");
        Set<String> features = new HashSet<String>();
        features.add("disableCallList");
        features.add("intercom");
        features.add("voiceQualityMonitoring");
        features.add("OTHERS_CodecPref");
        features.add("pre_3.2.0_model");
        model.setSupportedFeatures(features);
        phone.setModel(model);

        tester = PhoneTestDriver.supplyTestData(phone);
        initSettings();
        phone.setDeviceVersion(PolycomModel.VER_2_0);

        // XCF-3581: No longer automatically generating phone emergency dial routing. These
        // settings
        // are as if they'd been manually configured under Phone - Dial Plan - Emergency Routing.
        phone.setSettingValue("dialplan/digitmap/routing.1/address", "emergency-gateway.example.org");
        phone.setSettingValue("dialplan/digitmap/routing.1/port", "9999");
        phone.setSettingValue("dialplan/digitmap/routing.1/emergency.1.value", "911,912");

        // XX-6875: Polycom offers and accepts codecs that are de-selected. De-selected codecs
        // must be
        // explicitly generated with blank values. Otherwise the default values from the
        // polycom_sip.cfg
        // file are used, some of which are non-blank. The value below means G711Mu is
        // de-selected.
        phone.setSettingValue("voice/codecPref/OTHERS", "G711A|G729AB");

        phone.beforeProfileGeneration();
        ProfileContext cfg = new SipConfiguration(phone);

        m_pg.generate(m_location, cfg, null, "profile");

        InputStream expected = getClass().getResourceAsStream("expected-sip.cfg.xml");

        assertPolycomXmlEquals(expected, m_location.getReader());
        expected.close();
    }

    public void testGenerateVVX1500Profile() throws Exception {
        PolycomModel model = new PolycomModel();
        model.setMaxLineCount(6);
        model.setModelId("polycomVVX1500");
        Set<String> features = new HashSet<String>();
        features.add("disableCallList");
        features.add("intercom");
        features.add("voiceQualityMonitoring");
        features.add("nway-conference");
        features.add("localConferenceCallHold");
        features.add("singleKeyPressConference");
        features.add("VVX_1500_CodecPref");
        features.add("video");
        model.setSupportedFeatures(features);
        phone.setModel(model);
        tester = PhoneTestDriver.supplyTestData(phone);
        initSettings();
        phone.setDeviceVersion(PolycomModel.VER_2_0);
        // XCF-3581: No longer automatically generating phone emergency dial routing. These
        // settings
        // are as if they'd been manually configured under Phone - Dial Plan - Emergency Routing.
        phone.setSettingValue("dialplan/digitmap/routing.1/address", "emergency-gateway.example.org");
        phone.setSettingValue("dialplan/digitmap/routing.1/port", "9999");
        phone.setSettingValue("dialplan/digitmap/routing.1/emergency.1.value", "911,912");
        phone.setSettingValue("video/codecPref/videoCodecs", "H264|H2631998");//de-selected testing
        phone.beforeProfileGeneration();
        ProfileContext cfg = new SipConfiguration(phone);

        m_pg.generate(m_location, cfg, null, "profile");

        InputStream expected = getClass().getResourceAsStream("expected-VVX1500-sip.cfg.xml");

        assertPolycomXmlEquals(expected, m_location.getReader());

        expected.close();
    }

    private void initSettings() {
        // settings selected at random, there are too many
        // to test all. select a few.
        phone.setSettingValue("log/level.change/sip", "3");
        phone.setSettingValue("call/rejectBusyOnDnd", "0");
        phone.setSettingValue("voIpProt.SIP/local/port", "5061");
        phone.setSettingValue("call/rejectBusyOnDnd", "0");

        tester.getPrimaryLine().setSettingValue("call/serverMissedCall/enabled", "1");

        assertEquals("0", phone.getSettingValue("voIpProt.server.dhcp/available"));
    }
}
