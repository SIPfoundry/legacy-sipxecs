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
import java.util.Collections;
import java.util.LinkedHashSet;
import java.util.Set;

import junit.framework.TestCase;
import org.apache.commons.io.IOUtils;
import org.apache.commons.lang.StringUtils;
import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.device.MemoryProfileLocation;
import org.sipfoundry.sipxconfig.device.Profile;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.LineInfo;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.phone.PhoneModel;
import org.sipfoundry.sipxconfig.phone.PhoneTestDriver;
import org.sipfoundry.sipxconfig.phone.lg_nortel.LgNortelPhone.PhonebookProfile;
import org.sipfoundry.sipxconfig.speeddial.Button;
import org.sipfoundry.sipxconfig.speeddial.SpeedDial;


public class LgNortelPhoneTest extends TestCase {
    public void _testFactoryRegistered() {
        PhoneContext pc = (PhoneContext) TestHelper.getApplicationContext().getBean(
                PhoneContext.CONTEXT_BEAN_NAME);
        assertNotNull(pc.newPhone(new PhoneModel("lg-nortel")));
    }

    public void testGetFileName() throws Exception {
        LgNortelPhone phone = new LgNortelPhone();
        phone.setSerialNumber("0011aabb4455");
        assertEquals("0011AABB4455", phone.getProfileFilename());
    }

    public void testExternalLine() throws Exception {
        PhoneModel lgNortelModel = new PhoneModel("lg-nortel");
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
        PhoneModel lgNortelModel = new PhoneModel("lg-nortel");
        Phone phone = new LgNortelPhone();
        phone.setModel(lgNortelModel);

        PhoneTestDriver testDriver = PhoneTestDriver.supplyTestData(phone,true,false,false,true);
        phone.restart();

        testDriver.sipControl.verify();
    }

    public void testRestartNoLine() throws Exception {
        PhoneModel lgNortelModel = new PhoneModel("lg-nortel");
        Phone phone = new LgNortelPhone();
        phone.setModel(lgNortelModel);

        PhoneTestDriver testDriver = PhoneTestDriver.supplyTestData(phone, new ArrayList<User>(), true);

        phone.restart();

        testDriver.sipControl.verify();
    }

    /**
     * Tests that the phonebook profile is used when phonebook management is enabled.
     */
    public void testPhonebookManagementEnabled() throws Exception {
        PhoneModel lgNortelModel = new PhoneModel("lg-nortel");
        Phone phone = new LgNortelPhone();
        phone.setModel(lgNortelModel);
        PhoneTestDriver.supplyTestData(phone, true);

        // Should return two profiles - the regular profile and the phonebook
        // profile.
        Profile[] profileTypes = phone.getProfileTypes();
        assertEquals(2, profileTypes.length);
        assertTrue(profileTypes[0].getClass().equals(Profile.class));
        assertTrue(profileTypes[1].getClass().equals(PhonebookProfile.class));
    }

    /**
     * Tests that the phonebook profile is not used when phonebook management is disabled.
     */
    public void testPhonebookManagementDisabled() throws Exception {
        PhoneModel lgNortelModel = new PhoneModel("lg-nortel");
        Phone phone = new LgNortelPhone();
        phone.setModel(lgNortelModel);
        PhoneTestDriver.supplyTestData(phone, false);

        // Should only return one Profile.
        Profile[] profileTypes = phone.getProfileTypes();
        assertEquals(1, profileTypes.length);
        // Make sure it's not a PhonebookProfile. We can't use instanceof to
        // check the type, because since a PhonebookProfile is a Profile, the
        // result would be true. So we have to compare the classes directly.
        assertTrue(profileTypes[0].getClass().equals(Profile.class));
    }

    public void testGenerateTypicalProfile() throws Exception {
        PhoneModel lgNortelModel = new PhoneModel("lg-nortel");
        lgNortelModel.setProfileTemplate("lg-nortel/mac.cfg.vm");
        lgNortelModel.setMaxLineCount(4); // we are testing 2 lines
        LgNortelPhone phone = new LgNortelPhone();
        phone.setModel(lgNortelModel);

        MemoryProfileLocation location = TestHelper.setVelocityProfileGenerator(phone);

        supplyTestData(phone);

        phone.getProfileTypes()[0].generate(phone, location);
        InputStream expectedProfile = getClass().getResourceAsStream("mac.cfg");
        assertNotNull(expectedProfile);
        String expected = IOUtils.toString(expectedProfile);
        expectedProfile.close();

        assertEquals(expected, location.toString());
    }

    public void testGenerateProfileWithRouting() throws Exception {
        PhoneModel lgNortelModel = new PhoneModel("lg-nortel");
        lgNortelModel.setProfileTemplate("lg-nortel/mac.cfg.vm");
        lgNortelModel.setMaxLineCount(4); // we are testing 2 lines
        LgNortelPhone phone = new LgNortelPhone();
        phone.setModel(lgNortelModel);

        MemoryProfileLocation location = TestHelper.setVelocityProfileGenerator(phone);

        supplyTestData(phone);

        // XCF-3581: No longer automatically generating phone emergency dial routing.  These settings
        // are as if they'd been manually configured under Dialing.
        phone.setSettingValue("DIAL/emergency_number", "911");
        phone.setSettingValue("DIAL/emergency_address", "emergency-gateway.example.org");

        phone.getProfileTypes()[0].generate(phone, location);
        InputStream expectedProfile = getClass().getResourceAsStream("mac_e911.cfg");
        assertNotNull(expectedProfile);
        String expected = IOUtils.toString(expectedProfile);
        expectedProfile.close();

        assertEquals(expected, location.toString());
    }

    public void testGenerateLgNortel6804() throws Exception {
        PhoneModel lgNortelModel = new PhoneModel("lg-nortel");
        lgNortelModel.setProfileTemplate("lg-nortel/mac.cfg.vm");
        lgNortelModel.setMaxLineCount(4); // we are testing 2 lines
        lgNortelModel.setModelId("lip6804");
        LgNortelPhone phone = new LgNortelPhone();
        phone.setModel(lgNortelModel);

        MemoryProfileLocation location = TestHelper.setVelocityProfileGenerator(phone);

        supplyTestData(phone);

        phone.getProfileTypes()[0].generate(phone, location);
        InputStream expectedProfile = getClass().getResourceAsStream("mac_6804.cfg");
        assertNotNull(expectedProfile);
        String expected = IOUtils.toString(expectedProfile);
        expectedProfile.close();

        assertEquals(expected, location.toString());
    }

    public void testSpeeddial() throws Exception {
        Button[] buttons = new Button[] {
            new Button("Yogi", "yogi@example.com"), new Button("Daffy Duck", "213")
        };
        SpeedDial sp = new SpeedDial();
        sp.setButtons(Arrays.asList(buttons));

        PhoneModel lgNortelModel = new PhoneModel("lg-nortel");
        lgNortelModel.setProfileTemplate("lg-nortel/mac.cfg.vm");
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
            phone.getProfileTypes()[0].generate(phone, location);
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
            phone.getProfileTypes()[0].generate(phone, location);
            String actual = location.toString();
            String actualLines[] = StringUtils.split(actual, "\n");
            int i = find(actualLines, "[PROG]");
            assertTrue(i >= 0);
            assertEquals("add 2 1 yogi@example.com", actualLines[i + 1]);
            assertEquals("add 3 1 213", actualLines[i + 2]);
        }

        phoneContextControl.verify();
    }

    public void testGenerateProfileWithBlfSpeeddial() throws Exception {
        PhoneModel lgNortelModel = new PhoneModel("lg-nortel");
        lgNortelModel.setProfileTemplate("lg-nortel/mac.cfg.vm");
        lgNortelModel.setMaxLineCount(4);
        LgNortelPhone phone = new LgNortelPhone();
        phone.setModel(lgNortelModel);

        MemoryProfileLocation location = TestHelper.setVelocityProfileGenerator(phone);

        User user1 = new User() {
            @Override
            public Integer getId() {
                return 115;
            }
        };
        user1.setUserName("juser");
        user1.setFirstName("Joe");
        user1.setLastName("User");
        user1.setSipPassword("1234");
        PhoneTestDriver.supplyTestData(phone, Collections.singletonList(user1));

        User user2 = new User();
        User user3 = new User();
        user2.setUserName("Bill");
        user3.setUserName("Bob");

        Set user2Aliases = new LinkedHashSet(); // use LinkedHashSet for stable ordering
        user2Aliases.add("201");
        user2.setAliases(user2Aliases);

        Set user3Aliases = new LinkedHashSet(); // use LinkedHashSet for stable ordering
        user3Aliases.add("202");
        user3.setAliases(user3Aliases);

        Button[] buttons = new Button[] {
            new Button("Bill User", "201"), new Button("Bob User", "202")
        };

        SpeedDial sp = new SpeedDial();
        sp.setButtons(Arrays.asList(buttons));
        buttons[1].setBlf(true);
        sp.setUser(user1);

        IMocksControl coreContextControl = EasyMock.createNiceControl();
        CoreContext coreContext = coreContextControl.createMock(CoreContext.class);

        coreContext.loadUserByAlias("201");
        EasyMock.expectLastCall().andReturn(user2);
        coreContext.loadUserByAlias("202");
        EasyMock.expectLastCall().andReturn(user3);
        coreContextControl.replay();

        phone.setCoreContext(coreContext);

        IMocksControl phoneContextControl = EasyMock.createNiceControl();
        PhoneContext phoneContext = phoneContextControl.createMock(PhoneContext.class);
        PhoneTestDriver.supplyVitalTestData(phoneContextControl, phoneContext, phone);

        phoneContext.getSpeedDial(phone);
        phoneContextControl.andReturn(sp).anyTimes();
        phoneContextControl.replay();

        phone.getProfileTypes()[0].generate(phone, location);
        InputStream expectedProfile = getClass().getResourceAsStream("mac_blf.cfg");
        assertNotNull(expectedProfile);
        String expected = IOUtils.toString(expectedProfile);
        expectedProfile.close();

        assertEquals(expected, location.toString());
    }

    public void testDisableProvisionOverwrite() throws Exception {
        PhoneModel lgNortelModel = new PhoneModel("lg-nortel");
        lgNortelModel.setProfileTemplate("lg-nortel/mac.cfg.vm");
        lgNortelModel.setMaxLineCount(4); // we are testing 2 lines
        LgNortelPhone phone = new LgNortelPhone();
        phone.setModel(lgNortelModel);

        MemoryProfileLocation location = TestHelper.setVelocityProfileGenerator(phone);

        supplyTestData(phone);

        // disable PROVISION/overwrite
        phone.setSettingValue("PROVISION/overwrite", "0");
        phone.getProfileTypes()[0].generate(phone, location);
        InputStream expectedProfile = getClass().getResourceAsStream("mac_disable_overwrite_provision.cfg");
        assertNotNull(expectedProfile);
        String expected = IOUtils.toString(expectedProfile);
        expectedProfile.close();

        assertEquals(expected, location.toString());
    }
    public void testEnableProvisionOverwrite() throws Exception {
        PhoneModel lgNortelModel = new PhoneModel("lg-nortel");
        lgNortelModel.setProfileTemplate("lg-nortel/mac.cfg.vm");
        lgNortelModel.setMaxLineCount(4); // we are testing 2 lines
        LgNortelPhone phone = new LgNortelPhone();
        phone.setModel(lgNortelModel);

        MemoryProfileLocation location = TestHelper.setVelocityProfileGenerator(phone);

        supplyTestData(phone);

        // enable PROVISION/overwrite
        phone.setSettingValue("PROVISION/overwrite", "1");
        phone.getProfileTypes()[0].generate(phone, location);
        InputStream expectedProfile = getClass().getResourceAsStream("mac_enable_overwrite_provision.cfg");
        assertNotNull(expectedProfile);
        String expected = IOUtils.toString(expectedProfile);
        expectedProfile.close();

        assertEquals(expected, location.toString());
    }

    private void supplyTestData(LgNortelPhone phone) {
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

    private int find(String[] lines, String match) {
        for (int i = 0; i < lines.length; i++) {
            if (match.equals(lines[i])) {
                return i;
            }
        }
        return -1;
    }
}
