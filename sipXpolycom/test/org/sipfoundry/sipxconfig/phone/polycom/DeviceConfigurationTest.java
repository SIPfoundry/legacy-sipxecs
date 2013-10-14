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
import org.sipfoundry.sipxconfig.device.ProfileContext;
import org.sipfoundry.sipxconfig.device.ProfileGenerator;
import org.sipfoundry.sipxconfig.device.VelocityProfileGenerator;
import org.sipfoundry.sipxconfig.phone.PhoneTestDriver;
import org.sipfoundry.sipxconfig.test.MemoryProfileLocation;
import org.sipfoundry.sipxconfig.test.TestHelper;

public class DeviceConfigurationTest extends TestCase {

    private PolycomPhone phone;

    private ProfileGenerator m_pg;

    private MemoryProfileLocation m_location;

    @Override
    protected void setUp() throws Exception {
        PolycomModel model = PolycomXmlTestCase.phoneModelBuilder("polycom600", getClass());
        phone = new PolycomPhone();
        phone.setModel(model);
        PhoneTestDriver.supplyTestData(phone);
        phone.setDefaults(PhoneTestDriver.getDeviceDefaults());

        m_location = TestHelper.setVelocityProfileGenerator(phone, TestHelper.getEtcDir());
        m_pg = phone.getProfileGenerator();
    }

    public void testGenerateSyslogProfile() throws Exception {
        phone.setSettingValue("log/device.syslog/transport", "2");
        phone.setSettingValue("log/device.syslog/facility", "18");
        phone.setSettingValue("log/device.syslog/renderLevel", "5");
        phone.setSettingValue("log/device.syslog/prependMac", "0");
        phone.beforeProfileGeneration();
        ProfileContext cfg = new DeviceConfiguration(phone);

        m_pg.generate(m_location, cfg, null, "profile");

        InputStream expected = getClass().getResourceAsStream("expected-syslog-device.cfg.xml");

        assertEquals(IOUtils.toString(expected), m_location.toString());
        expected.close();
    }

    public void testGenerateNetworkProfileDisableOverwrite() throws Exception {
        phone.setSettingValue("network/device.net/overwrite", "0");
        phone.beforeProfileGeneration();
        ProfileContext cfg = new DeviceConfiguration(phone);

        m_pg.generate(m_location, cfg, null, "profile");

        InputStream expected = getClass().getResourceAsStream("expected-disable-overwrite-device.cfg.xml");

        assertEquals(IOUtils.toString(expected), m_location.toString());
        expected.close();
    }

    public void testGenerateNetworkProfileEnableOverwrite() throws Exception {
        phone.setSettingValue("network/device.net/overwrite", "1");
        phone.beforeProfileGeneration();
        ProfileContext cfg = new DeviceConfiguration(phone);

        m_pg.generate(m_location, cfg, null, "profile");

        InputStream expected = getClass().getResourceAsStream("expected-enable-overwrite-device.cfg.xml");

        assertEquals(IOUtils.toString(expected), m_location.toString());
        expected.close();
    }
}
