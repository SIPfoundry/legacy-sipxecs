/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxconfig.phone.aastra;

import java.io.InputStream;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import junit.framework.TestCase;

import org.apache.commons.io.IOUtils;
import org.apache.commons.lang.StringUtils;
import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.device.MemoryProfileLocation;
import org.sipfoundry.sipxconfig.device.Profile;
import org.sipfoundry.sipxconfig.device.RestartException;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.LineInfo;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.phone.PhoneModel;
import org.sipfoundry.sipxconfig.phone.PhoneTestDriver;
import org.sipfoundry.sipxconfig.speeddial.Button;
import org.sipfoundry.sipxconfig.speeddial.SpeedDial;
import org.sipfoundry.sipxconfig.phone.aastra.AastraPhone.PhonebookProfile;

public class AastraPhoneTest extends TestCase {

    public void testGetFileName() throws Exception {
        PhoneModel aastraModel = new PhoneModel("aastra");
        AastraPhone phone = new AastraPhone();
        phone.setModel(aastraModel);
        supplyTestData(phone);

        phone.setSerialNumber("0011AABB4050");
        Profile[] profileTypes = phone.getProfileTypes();
        assertEquals(2, profileTypes.length);
        assertEquals("0011AABB4050.cfg", profileTypes[0].getName());
    }

    public void testExternalLine() throws Exception {
        PhoneModel AastraModel = new PhoneModel("aastra");
        Phone phone = new AastraPhone();
        phone.setModel(AastraModel);

        PhoneTestDriver.supplyTestData(phone, new ArrayList<User>());
        LineInfo li = new LineInfo();
        li.setDisplayName("John Alan");
        li.setUserId("flast");
        li.setRegistrationServer("example.org");
        li.setPassword("12345");

        Line line = phone.createLine();
        phone.addLine(line);
        line.setLineInfo(li);

        assertEquals("\"John Alan\"<sip:flast@example.org>", line.getUri());
    }

    public void testRestart() throws Exception {
        PhoneModel aastraModel = new PhoneModel("aastra");
        Phone phone = new AastraPhone();
        phone.setModel(aastraModel);

        PhoneTestDriver testDriver = PhoneTestDriver.supplyTestData(phone);
        phone.restart();

        testDriver.sipControl.verify();
    }

    public void testRestartNoLine() throws Exception {
        PhoneModel aastraModel = new PhoneModel("aastra");
        Phone phone = new AastraPhone();
        phone.setModel(aastraModel);

        PhoneTestDriver.supplyTestData(phone, new ArrayList<User>());
        try {
            phone.restart();
            fail();
        } catch (RestartException re) {
            assertTrue(true);
        }
    }

    // FIXME: this test is failing - no speeddial generated
    public void _testSpeeddialWithLines() throws Exception {

        Button[] buttons = new Button[] {
            new Button("jeo", "jeo@example.com"), new Button("Daffy Duck", "130")
        };
        SpeedDial sp = new SpeedDial();
        sp.setButtons(Arrays.asList(buttons));

        PhoneModel aastraModel = new PhoneModel("aastra");
        aastraModel.setProfileTemplate("aastra/aastra.cfg.vm");
        Phone phone = new AastraPhone();
        phone.setModel(aastraModel);
        IMocksControl phoneContextControl = EasyMock.createNiceControl();
        PhoneContext phoneContext = phoneContextControl.createMock(PhoneContext.class);
        PhoneTestDriver.supplyVitalTestData(phoneContextControl, phoneContext, phone);

        phoneContext.getSpeedDial(phone);
        phoneContextControl.andReturn(sp).anyTimes();

        phoneContextControl.replay();

        Line line = phone.createLine();
        User user = new User();
        user.setUserName("juser");
        line.setUser(user);
        phone.addLine(line);

        MemoryProfileLocation location = TestHelper.setVelocityProfileGenerator(phone);
        phone.getProfileTypes()[0].generate(phone, location);
        String actual = location.toString();
        String actualLines[] = StringUtils.split(actual, "\n");

        int i = find(actualLines, "[PROG]");
        assertTrue(i >= 0);
        assertEquals("add 2 1 jeo@example.com", actualLines[i + 1]);
        assertEquals("add 3 1 130", actualLines[i + 2]);

        phoneContextControl.verify();
    }

    // FIXME: this test is failing - no speeddial generated
    public void _testSpeeddialNoLines() throws Exception {
        Button[] buttons = new Button[] {
            new Button("jeo", "jeo@example.com"), new Button("Daffy Duck", "130")
        };
        SpeedDial sp = new SpeedDial();
        sp.setButtons(Arrays.asList(buttons));

        PhoneModel aastraModel = new PhoneModel("aastra");
        aastraModel.setProfileTemplate("aastra/aastra.cfg.vm");
        Phone phone = new AastraPhone();
        phone.setModel(aastraModel);
        IMocksControl phoneContextControl = EasyMock.createNiceControl();
        PhoneContext phoneContext = phoneContextControl.createMock(PhoneContext.class);
        PhoneTestDriver.supplyVitalTestData(phoneContextControl, phoneContext, phone);

        phoneContext.getSpeedDial(phone);
        phoneContextControl.andReturn(sp).anyTimes();

        phoneContextControl.replay();

        MemoryProfileLocation location = TestHelper.setVelocityProfileGenerator(phone);
        phone.getProfileTypes()[0].generate(phone, location);
        String actual = location.toString();
        System.err.println(actual);
        String actualLines[] = StringUtils.split(actual, "\n");
        int i = find(actualLines, "[PROG]");
        assertTrue(i >= 0);
        assertEquals("add 1 1 jeo@example.com", actualLines[i + 1]);
        assertEquals("add 2 1 130", actualLines[i + 2]);

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

    public void testGenerateTypicalProfile() throws Exception {
        PhoneModel aastraModel = new PhoneModel("aastra");
        aastraModel.setProfileTemplate("aastra/aastra.cfg.vm");
        aastraModel.setMaxLineCount(9);
        AastraPhone phone = new AastraPhone();
        phone.setModel(aastraModel);

        MemoryProfileLocation location = TestHelper.setVelocityProfileGenerator(phone);

        supplyTestData(phone);

        phone.setSerialNumber("0011AABB4050");
        phone.getProfileTypes()[0].generate(phone, location);

        assertEquals(1, phone.getProfileTypes().length);
        assertEquals("0011AABB4050.cfg", phone.getProfileTypes()[0].getName());

        InputStream expectedProfile = getClass().getResourceAsStream("mac.cfg");

        assertNotNull(expectedProfile);
        assertEquals(IOUtils.toString(expectedProfile), location.toString());
    }

    public void testGenerateProfileWithNoLines() throws Exception {
        List<User> m_lines = new ArrayList<User>();
        PhoneModel aastraModel = new PhoneModel("aastra");
        aastraModel.setProfileTemplate("aastra/aastra.cfg.vm");
        aastraModel.setMaxLineCount(9);
        AastraPhone phone = new AastraPhone();
        phone.setModel(aastraModel);

        MemoryProfileLocation location = TestHelper.setVelocityProfileGenerator(phone);

        PhoneTestDriver.supplyTestData(phone, m_lines);

        phone.getProfileTypes()[0].generate(phone, location);

        InputStream expectedProfile = getClass().getResourceAsStream("mac_noline.cfg");

        assertNotNull(expectedProfile);
        assertEquals(IOUtils.toString(expectedProfile), location.toString());
    }

    private void supplyTestData(AastraPhone phone) {
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
    }

    public void testPhonebookName() throws Exception {
        PhoneModel aastraModel = new PhoneModel("aastra");
        AastraPhone phone = new AastraPhone();
        phone.setModel(aastraModel);

        PhoneTestDriver.supplyTestData(phone, true);

        phone.setSerialNumber("0011AABB4050");
        assertEquals(2, phone.getProfileTypes().length);
        assertEquals("0011AABB4050.cfg", phone.getProfileFilename());
        assertEquals("0011AABB4050-Directory.csv", phone.getPhonebookFilename());
    }

    public void testPhonebookNameDefault() throws Exception {
        PhoneModel aastraModel = new PhoneModel("aastra");
        AastraPhone phone = new AastraPhone();
        phone.setModel(aastraModel);

        PhoneTestDriver.supplyTestData(phone, true);

        assertEquals(2, phone.getProfileTypes().length);
        String phonebookName = phone.getSerialNumber().toUpperCase() + "-Directory.csv";
        assertEquals(phone.getProfileTypes()[0].getName(), phone.getProfileFilename());
        assertEquals(phonebookName, phone.getPhonebookFilename());
    }

    public void testPhonebookManagementEnabled() throws Exception {
        PhoneModel aastraModel = new PhoneModel("aastra");
        Phone phone = new AastraPhone();
        phone.setModel(aastraModel);
        PhoneTestDriver.supplyTestData(phone, true);

        // Should return two profiles - the regular profile and the phonebook
        // profile.
        Profile[] profileTypes = phone.getProfileTypes();
        assertEquals(2, profileTypes.length);
        assertTrue(profileTypes[0].getClass().equals(Profile.class));
        assertTrue(profileTypes[1].getClass().equals(PhonebookProfile.class));
    }

    public void testPhonebookManagementDisabled() throws Exception {
        PhoneModel aastraModel = new PhoneModel("aastra");
        Phone phone = new AastraPhone();
        phone.setModel(aastraModel);
        PhoneTestDriver.supplyTestData(phone, false);

        // Should only return one Profile.
        Profile[] profileTypes = phone.getProfileTypes();
        assertEquals(1, profileTypes.length);
        assertTrue(profileTypes[0].getClass().equals(Profile.class));
    }
}
