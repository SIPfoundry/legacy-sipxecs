/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.phone.counterpath;

import static org.easymock.EasyMock.expectLastCall;
import static org.sipfoundry.sipxconfig.test.TestUtil.getModelDirectory;

import java.util.ArrayList;
import java.util.List;

import junit.framework.TestCase;

import org.apache.commons.io.IOUtils;
import org.easymock.EasyMock;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.device.DeviceDefaults;
import org.sipfoundry.sipxconfig.device.MemoryProfileLocation;
import org.sipfoundry.sipxconfig.permission.PermissionManagerImpl;
import org.sipfoundry.sipxconfig.permission.PermissionName;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.phone.PhoneTestDriver;
import org.sipfoundry.sipxconfig.phone.counterpath.CounterpathPhone.CounterpathLineDefaults;
import org.sipfoundry.sipxconfig.phone.counterpath.CounterpathPhone.CounterpathPhoneDefaults;
import org.sipfoundry.sipxconfig.speeddial.Button;
import org.sipfoundry.sipxconfig.speeddial.SpeedDial;

public class CounterpathPhoneTest extends TestCase {
    private Line m_line;
    private User m_user;
    private CounterpathPhone m_phone;
    private PermissionManagerImpl m_permissionManager;

    protected void setUp() {
        CounterpathPhoneModel counterpathModel = new CounterpathPhoneModel("counterpath");
        counterpathModel.setProfileTemplate("counterpath/counterpath.ini.vm");
        counterpathModel.setModelId("counterpathCMCEnterprise");
        counterpathModel.setMaxLineCount(5);
        m_phone = new CounterpathPhone();
        m_phone.setModel(counterpathModel);
        m_phone.setDefaults(new DeviceDefaults());

        m_permissionManager = new PermissionManagerImpl();
        m_permissionManager.setModelFilesContext(TestHelper.getModelFilesContext(getModelDirectory("neoconf")));
    }

    public void testGetFileName() throws Exception {
        m_phone.setSerialNumber("0011AABB4455");
        assertEquals("0011AABB4455.ini", m_phone.getProfileFilename());
    }

    private class LocationMock extends Location {
        public String getFqdn() {
            return "fqdn.im.test.box";
        }
    }

    public void testGenerateCounterpathCMCEnterprise() throws Exception {
        PhoneTestDriver.supplyTestData(m_phone, true, true, true, true);
        User firstUser = m_phone.getLines().get(0).getUser();
        firstUser.setPermissionManager(m_permissionManager);
        firstUser.setImId("jsmit_id");
        firstUser.setImDisplayName("John Smith Id");
        firstUser.getSettings().getSetting("im/im-account").setValue("1");

        User secondUser = m_phone.getLines().get(1).getUser();
        secondUser.setPermissionManager(m_permissionManager);
        secondUser.setImId("sharedUser_id");
        secondUser.setImDisplayName("Shared User Id");
        m_phone.getLines().get(1).getSettings().getSetting("presence/workgroup/allow_dialog_subscriptions").setValue("0");

        Location locationMock = new LocationMock();
        LocationsManager locationsManagerMock = EasyMock.createMock(LocationsManager.class);
        locationsManagerMock.getLocationByBundle("imBundle");
        EasyMock.expectLastCall().andReturn(locationMock);
        locationsManagerMock.getPrimaryLocation();
        expectLastCall().andReturn(locationMock).anyTimes();
        EasyMock.replay(locationsManagerMock);
        m_phone.setLocationsManager(locationsManagerMock);

        MemoryProfileLocation location = TestHelper.setVelocityProfileGenerator(m_phone);
        m_phone.generateProfiles(location);
        String expected = IOUtils.toString(getClass().getResourceAsStream("cmc-enterprise.ini"));
        System.out.println("((((" + expected + "))))");
        System.out.println("((((" + location.toString(m_phone.getPhoneFilename()) + "))))");

        assertEquals(expected, location.toString(m_phone.getPhoneFilename()));



        expected = IOUtils.toString(getClass().getResourceAsStream("contactlist.xml"));

        System.out.println("Directory :((((" + expected + "))))");
        System.out.println("((((" + location.toString(m_phone.getDirectoryFilename()) + "))))");

        assertEquals(expected, location.toString(m_phone.getDirectoryFilename()));
    }

    public void testCounterpathLineDefaults() {
        DeviceDefaults defaults = new DeviceDefaults();
        defaults.setDomainManager(TestHelper.getTestDomainManager("example.org"));
        m_phone.setDefaults(defaults);

        m_line = m_phone.createLine();
        supplyUserData();
        m_line.setUser(m_user);

        CounterpathLineDefaults lineDefaults = new CounterpathPhone.CounterpathLineDefaults(m_line);

        PhoneContext phoneContextMock = EasyMock.createMock(PhoneContext.class);
        phoneContextMock.getPhoneDefaults();
        EasyMock.expectLastCall().andReturn(defaults).anyTimes();
        EasyMock.replay(phoneContextMock);

        m_phone.setPhoneContext(phoneContextMock);

        assertEquals("jsmit", lineDefaults.getUserName());
        assertEquals("John Smit", lineDefaults.getDisplayName());
        assertEquals("1234", lineDefaults.getPassword());
        assertEquals("example.org", lineDefaults.getDomain());
        assertEquals("101", lineDefaults.getVoicemailURL());
    }

    public void testCounterpathPhoneDefaults() {
        SpeedDial speedDial = new SpeedDial();
        supplyUserData();
        speedDial.setUser(m_user);

        DeviceDefaults defaults = new DeviceDefaults();
        defaults.setDomainManager(TestHelper.getTestDomainManager("example.org"));

        PhoneContext phoneContextMock = EasyMock.createMock(PhoneContext.class);
        phoneContextMock.getSpeedDial(m_phone);
        EasyMock.expectLastCall().andReturn(speedDial).anyTimes();
        phoneContextMock.getPhoneDefaults();
        EasyMock.expectLastCall().andReturn(defaults).anyTimes();
        EasyMock.replay(phoneContextMock);

        m_phone.setPhoneContext(phoneContextMock);

        CounterpathPhoneDefaults phoneDefaults = m_phone.new CounterpathPhoneDefaults(m_phone);

    }

    public void testCounterpathWithBLFSpeeddials() {
        SpeedDial speedDial = new SpeedDial();
        supplyUserData();
        List<Button> buttons = new ArrayList<Button>();
        Button button1 = new Button("test_button_one", "1000");
        button1.setBlf(true);
        buttons.add(button1);
        speedDial.setButtons(buttons);
        speedDial.setUser(m_user);

        DeviceDefaults defaults = new DeviceDefaults();
        defaults.setDomainManager(TestHelper.getTestDomainManager("example.org"));

        PhoneContext phoneContextMock = EasyMock.createMock(PhoneContext.class);
        phoneContextMock.getSpeedDial(m_phone);
        EasyMock.expectLastCall().andReturn(speedDial).anyTimes();
        phoneContextMock.getPhoneDefaults();
        EasyMock.expectLastCall().andReturn(defaults).anyTimes();
        EasyMock.replay(phoneContextMock);

        m_phone.setPhoneContext(phoneContextMock);

        CounterpathPhoneDefaults phoneDefaults = m_phone.new CounterpathPhoneDefaults(m_phone);

        assertEquals("sip:~~rl~C~jsmit@example.org", phoneDefaults.getWorkgroupSubscriptionAor());
    }

    private void supplyUserData() {
        m_user = new User();
        m_user.setPermissionManager(m_permissionManager);
        m_user.setUserName("jsmit");
        m_user.setFirstName("John");
        m_user.setLastName("Smit");
        m_user.setSipPassword("1234");
        m_user.setImId("jsmit_id");
        m_user.setImDisplayName("John Smith Id");
    }

    public void testGenerateCounterpathCMCEnterpriseWithoutVoicemailPermission() throws Exception {
        List<User> users = new ArrayList<User>();

        PermissionManagerImpl pManager = new PermissionManagerImpl();
        pManager.setModelFilesContext(TestHelper.getModelFilesContext(getModelDirectory("neoconf")));

        User user1 = new User();
        user1.setUserName("juser");
        user1.setFirstName("Joe");
        user1.setLastName("User");
        user1.setSipPassword("1234");
        user1.setPermissionManager(pManager);
        user1.setPermission(PermissionName.VOICEMAIL, false);

        User user2 = new User();
        user2.setUserName("kuser");
        user2.setFirstName("Kate");
        user2.setLastName("User");
        user2.setSipPassword("1234");
        user2.setPermissionManager(pManager);
        user2.setPermission(PermissionName.VOICEMAIL, true);

        users.add(user1);
        users.add(user2);

        PhoneTestDriver.supplyTestData(m_phone, users);
        User firstUser = m_phone.getLines().get(0).getUser();
        firstUser.setPermissionManager(m_permissionManager);
        firstUser.setImId("jsmit_id");
        firstUser.setImDisplayName("John Smith Id");
        firstUser.getSettings().getSetting("im/im-account").setValue("1");

        Location locationMock = new LocationMock();
        LocationsManager locationsManagerMock = EasyMock.createMock(LocationsManager.class);
        locationsManagerMock.getLocationByBundle("imBundle");
        EasyMock.expectLastCall().andReturn(locationMock);
        locationsManagerMock.getPrimaryLocation();
        expectLastCall().andReturn(locationMock).anyTimes();
        EasyMock.replay(locationsManagerMock);
        m_phone.setLocationsManager(locationsManagerMock);

        MemoryProfileLocation location = TestHelper.setVelocityProfileGenerator(m_phone);
        m_phone.generateProfiles(location);
        String expected = IOUtils.toString(getClass().getResourceAsStream("cmc-enterprise-without-voicemail-permission.ini"));

        assertEquals(expected, location.toString(m_phone.getPhoneFilename()));
    }
}
