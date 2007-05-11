/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.phone.lg_nortel;

import java.io.InputStream;
import java.util.ArrayList;
import java.util.Arrays;

import junit.framework.TestCase;

import org.apache.commons.io.IOUtils;
import org.apache.commons.lang.StringUtils;
import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.device.MemoryProfileLocation;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.LineInfo;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.phone.PhoneTestDriver;
import org.sipfoundry.sipxconfig.phone.RestartException;
import org.sipfoundry.sipxconfig.speeddial.Button;
import org.sipfoundry.sipxconfig.speeddial.SpeedDial;

public class LgNortelPhoneTest extends TestCase {
    public void _testFactoryRegistered() {
        PhoneContext pc = (PhoneContext) TestHelper.getApplicationContext().getBean(
                PhoneContext.CONTEXT_BEAN_NAME);
        assertNotNull(pc.newPhone(new LgNortelModel()));
    }

    public void testGetFileName() throws Exception {
        LgNortelPhone phone = new LgNortelPhone();
        phone.setSerialNumber("0011aabb4455");
        assertEquals("0011AABB4455", phone.getProfileFilename());
    }

    public void testExternalLine() throws Exception {
        LgNortelModel lgNortelModel = new LgNortelModel();
        Phone phone = new LgNortelPhone();
        phone.setModel(lgNortelModel);

        PhoneTestDriver.supplyTestData(phone, new ArrayList<User>());
        LineInfo li = new LineInfo();
        li.setDisplayName("First Last");
        li.setUserId("flast");
        li.setRegistrationServer("example.org");
        li.setPassword("12345");

        Line line = phone.createLine();
        phone.addLine(line);
        line.setLineInfo(li);

        assertEquals("\"First Last\"<sip:flast@example.org>", line.getUri());
    }

    public void testRestart() throws Exception {
        LgNortelModel lgNortelModel = new LgNortelModel();
        Phone phone = new LgNortelPhone();
        phone.setModel(lgNortelModel);

        PhoneTestDriver testDriver = PhoneTestDriver.supplyTestData(phone);
        phone.restart();

        testDriver.sipControl.verify();
    }

    public void testRestartNoLine() throws Exception {
        LgNortelModel lgNortelModel = new LgNortelModel();
        Phone phone = new LgNortelPhone();
        phone.setModel(lgNortelModel);

        PhoneTestDriver.supplyTestData(phone, new ArrayList<User>());
        try {
            phone.restart();
            fail();
        } catch (RestartException re) {
            assertTrue(true);
        }
    }

    public void testGenerateTypicalProfile() throws Exception {
        LgNortelModel lgNortelModel = new LgNortelModel();
        lgNortelModel.setMaxLineCount(4); // we are testing 2 lines
        LgNortelPhone phone = new LgNortelPhone();
        phone.setModel(lgNortelModel);

        MemoryProfileLocation location = TestHelper.setVelocityProfileGenerator(phone);

        User u1 = new User();
        u1.setUserName("juser");
        u1.setFirstName("Joe");
        u1.setLastName("User");
        u1.setSipPassword("1234");

        User u2 = new User();
        u2.setUserName("buser");
        u2.setSipPassword("abcdef");
        u2.addAlias("432");

        // call this to inject dummy data
        PhoneTestDriver.supplyTestData(phone, Arrays.asList(new User[] {
            u1, u2
        }));

        phone.generateProfiles(location);
        InputStream expectedProfile = getClass().getResourceAsStream("mac.cfg");
        assertNotNull(expectedProfile);
        String expected = IOUtils.toString(expectedProfile);
        expectedProfile.close();

        String actual = location.toString();

        String expectedLines[] = StringUtils.split(expected, "\n");
        String actualLines[] = StringUtils.split(actual, "\n");

        int len = Math.min(actualLines.length, expectedLines.length);
        for (int i = 0; i < len; i++) {
            assertEquals(expectedLines[i], actualLines[i]);
        }
        assertEquals(expectedLines.length, actualLines.length);
    }

    public void testSpeeddial() throws Exception {
        Button[] buttons = new Button[] {
            new Button("Yogi", "yogi@example.com"), new Button("Daffy Duck", "213")
        };
        SpeedDial sp = new SpeedDial();
        sp.setButtons(Arrays.asList(buttons));

        LgNortelModel lgNortelModel = new LgNortelModel();
        Phone phone = new LgNortelPhone();
        phone.setModel(lgNortelModel);
        IMocksControl phoneContextControl = EasyMock.createNiceControl();
        PhoneContext phoneContext = phoneContextControl.createMock(PhoneContext.class);
        PhoneTestDriver.supplyVitalTestData(phoneContextControl, phoneContext, phone);

        phoneContext.getSpeedDial(phone);
        phoneContextControl.andReturn(sp).anyTimes();

        phoneContextControl.replay();

        {
            MemoryProfileLocation location = TestHelper.setVelocityProfileGenerator(phone);
            phone.generateProfiles(location);
            String actual = location.toString();
            String actualLines[] = StringUtils.split(actual, "\n");
            int i = find(actualLines, "[PROG]");
            assertTrue(i >= 0);
            assertEquals("add 1 1 yogi@example.com", actualLines[i + 1]);
            assertEquals("add 2 1 213", actualLines[i + 2]);
        }

        Line line = phone.createLine();
        User user = new User();
        user.setUserName("juser");
        line.setUser(user);
        phone.addLine(line);
        {
            MemoryProfileLocation location = TestHelper.setVelocityProfileGenerator(phone);
            phone.generateProfiles(location);
            String actual = location.toString();
            String actualLines[] = StringUtils.split(actual, "\n");
            int i = find(actualLines, "[PROG]");
            assertTrue(i >= 0);
            assertEquals("add 2 1 yogi@example.com", actualLines[i + 1]);
            assertEquals("add 3 1 213", actualLines[i + 2]);
        }

        phoneContextControl.verify();
    }

    private int find(String[] lines, String match) {
        for (int i = 0; i < lines.length; i++) {
            if (match.equals(lines[i])) {
                return i;
            }
        }
        return -1;
    }
}
