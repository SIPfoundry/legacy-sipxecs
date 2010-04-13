/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.phone.cisco;

import java.util.List;

import junit.framework.TestCase;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.device.MemoryProfileLocation;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.LineInfo;
import org.sipfoundry.sipxconfig.phone.PhoneTestDriver;

public class CiscoIpPhoneTest extends TestCase {

    private CiscoIpPhone m_phone;

    protected void setUp() {
        CiscoModel model = new CiscoModel("ciscoIp");
        model.setMaxLineCount(6);
        model.setModelId("cisco7960");
        model.setProfileTemplate("ciscoIp/cisco-ip.vm");
        m_phone = new CiscoIpPhone();
        m_phone.setModel(model);
        PhoneTestDriver.supplyTestData(m_phone);
    }

    public void testGetSettings() {
        m_phone.getSettings();
    }

    public void testGenerate7960Profiles() throws Exception {
        MemoryProfileLocation location = TestHelper.setVelocityProfileGenerator(m_phone);
        PhoneTestDriver.supplyVitalEmergencyData(m_phone, "sos");
        m_phone.generateProfiles(location);
        String expected = IOUtils.toString(this.getClass().getResourceAsStream(
                "expected-7960.cfg"));
        assertEquals(expected, location.toString());
    }

    public void testAddExternalLine() {
        LineInfo lineInfo = new LineInfo();
        lineInfo.setDisplayName("Joe User");
        lineInfo.setUserId("juser");
        lineInfo.setPassword("sippassword");

        Line line = m_phone.createLine();
        m_phone.addLine(line);
        line.setLineInfo(lineInfo);

        List<Line> lines = m_phone.getLines();
        assertEquals(2, lines.size());

        Line externalLine = lines.get(1);
        assertEquals("sippassword", externalLine.getSettingTypedValue("line/password"));
        assertEquals("juser", externalLine.getSettingTypedValue("line/name"));
        assertEquals("Joe User", externalLine.getSettingTypedValue("line/displayname"));
    }
}
