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

import junit.framework.TestCase;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.device.MemoryProfileLocation;
import org.sipfoundry.sipxconfig.device.ProfileContext;
import org.sipfoundry.sipxconfig.device.ProfileGenerator;
import org.sipfoundry.sipxconfig.device.VelocityProfileGenerator;
import org.sipfoundry.sipxconfig.phone.PhoneTestDriver;

public class SipConfigurationTest extends TestCase {

    private PolycomPhone phone;

    private PhoneTestDriver tester;

    private ProfileGenerator m_pg;

    private MemoryProfileLocation m_location;

    @Override
    protected void setUp() throws Exception {
        PolycomModel model = new PolycomModel();
        model.setMaxLineCount(6);
        model.setModelId("polycom600");
        model.setQualityMonitoringSupported(true);
        phone = new PolycomPhone();
        phone.setModel(model);
        tester = PhoneTestDriver.supplyTestData(phone);

        m_location = new MemoryProfileLocation();
        VelocityProfileGenerator pg = new VelocityProfileGenerator();
        pg.setVelocityEngine(TestHelper.getVelocityEngine());
        m_pg = pg;
    }

    public void testGenerateProfile20() throws Exception {
        initSettings();
        phone.setDeviceVersion(PolycomModel.VER_2_0);
        phone.setSettingValue("dialplan/digitmap/routing.1/emergency.1.value", "911,912");
        phone.beforeProfileGeneration();
        ProfileContext cfg = new SipConfiguration(phone);

        m_pg.generate(m_location, cfg, null, "profile");

        InputStream expected = getClass().getResourceAsStream("expected-sip.cfg.xml");

        assertEquals(IOUtils.toString(expected), m_location.toString());
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
