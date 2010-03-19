/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxconfig.admin.dialplan.attendant;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.sipfoundry.sipxconfig.admin.AbstractConfigurationFile.getFileContent;

import java.io.InputStream;
import java.util.ArrayList;
import java.util.Arrays;

import org.sipfoundry.sipxconfig.common.DaoUtils;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.acd.BeanWithSettingsTestCase;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.conference.Conference;
import org.sipfoundry.sipxconfig.conference.ConferenceBridgeContext;
import org.sipfoundry.sipxconfig.phonebook.Address;
import org.sipfoundry.sipxconfig.phonebook.AddressBookEntry;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingImpl;
import org.sipfoundry.sipxconfig.setting.SettingSet;


public class ContactInformationConfigTest extends BeanWithSettingsTestCase {
    private static final String USER_CONSTANT = "user";
    private static final String IM_NOTIFICATION_SETTING = "im_notification";
    private static final String CONFERENCE_ENTRY_IM_SETTING = "conferenceEntryIM";
    private static final String CONFERENCE_EXIT_IM_SETTING = "conferenceExitIM";
    private static final String LEAVE_MSG_BEGIN_IM_SETTING = "leaveMsgBeginIM";
    private static final String LEAVE_MSG_END_IM_SETTING = "leaveMsgEndIM";

    private final Conference m_conf = new Conference();
    private User m_u1 = new User();
    private User m_u2 = new User();
    private User m_u3 = new User();
    private User m_u4 = new User();

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        initializeBeanWithSettings(m_conf);
        addImNotificationSettings(m_u1);
        addImNotificationSettings(m_u2);
        addImNotificationSettings(m_u3);
        addImNotificationSettings(m_u4);
    }

    public void testGenerate() throws Exception {
        m_u1.setUserName("200");
        m_u1.setFirstName("Ilya");
        m_u1.setLastName("Kovalchuk");
        AddressBookEntry abe1 = new AddressBookEntry();
        abe1.setAlternateImId("ilya");
        abe1.setAssistantName("Vyacheslav Kozlov");
        abe1.setAssistantPhoneNumber("0040721234567");
        abe1.setCellPhoneNumber("00407219874563");
        abe1.setCompanyName("Atlanta Thrashers");
        abe1.setFaxNumber("004021321654987");
        abe1.setHomePhoneNumber("0040216534582");
        abe1.setImDisplayName("Kovalchuk17");
        abe1.setImId("ik");
        abe1.setJobDept("Forwards");
        abe1.setJobTitle("Captain");
        abe1.setLocation("Field");
        Address homeAddress1 = new Address();
        homeAddress1.setCity("Atlanta");
        homeAddress1.setStreet("Merrivale Road");
        homeAddress1.setCountry("USA");
        homeAddress1.setState("GA");
        homeAddress1.setZip("90210");
        Address officeAddress1 = new Address();
        officeAddress1.setStreet("Kent Street");
        officeAddress1.setCity("Atlanta");
        officeAddress1.setState("GA");
        officeAddress1.setCountry("USA");
        officeAddress1.setZip("90211");
        officeAddress1.setOfficeDesignation("17");
        abe1.setHomeAddress(homeAddress1);
        abe1.setOfficeAddress(officeAddress1);

        m_u1.setAddressBookEntry(abe1);
        setImNotificationSettings(m_u1, "true", "false", "false", "false");

        m_conf.setOwner(m_u1);
        m_conf.setName("conf1");
        m_conf.setExtension("111");
        m_conf.getSettings().getSetting(Conference.PARTICIPANT_CODE).setValue("1234");

        m_u2.setUserName("201");
        AddressBookEntry abe2 = new AddressBookEntry();
        m_u2.setAddressBookEntry(abe2);
        setImNotificationSettings(m_u2, "true", "false", "false", "false");

        m_u4.setUserName("Jagr");
        m_u4.addAlias("nhlPlayer");

        AddressBookEntry abe4 = new AddressBookEntry();
        m_u4.setAddressBookEntry(abe4);

        setImNotificationSettings(m_u4, "true", "false", "false", "false");

        CoreContext coreContext = createMock(CoreContext.class);
        coreContext.loadUsersByPage(0, DaoUtils.PAGE_SIZE);
        expectLastCall().andReturn(Arrays.asList(m_u1, m_u2, m_u4));


        ConferenceBridgeContext bridgeContext = createMock(ConferenceBridgeContext.class);
        bridgeContext.findConferencesByOwner(m_u1);

        expectLastCall().andReturn(Arrays.asList(m_conf)).once().andReturn(new ArrayList<Conference>()).times(3);
        replay(bridgeContext);
        replay(coreContext);

        ContactInformationConfig cicfg = new ContactInformationConfig();
        cicfg.setCoreContext(coreContext);
        cicfg.setConferenceBridgeContext(bridgeContext);

        String generatedXml = getFileContent(cicfg, null);
        InputStream referenceXml = getClass().getResourceAsStream("contact-information.test.xml");
        assertEquals(IOUtils.toString(referenceXml), generatedXml);
    }

    private void addImNotificationSettings(User user) {
        Setting userSetting = new SettingSet(USER_CONSTANT);
        Setting userPaSetting = new SettingSet(IM_NOTIFICATION_SETTING);
        Setting userConferenceEntryIMSetting = new SettingImpl(CONFERENCE_ENTRY_IM_SETTING);
        Setting userConferenceExitIMSetting = new SettingImpl(CONFERENCE_EXIT_IM_SETTING);
        Setting userLeaveMsgBeginIMSetting = new SettingImpl(LEAVE_MSG_BEGIN_IM_SETTING);
        Setting userLeaveMsgEndIMSetting = new SettingImpl(LEAVE_MSG_END_IM_SETTING);
        userPaSetting.addSetting(userConferenceEntryIMSetting);
        userPaSetting.addSetting(userConferenceExitIMSetting);
        userPaSetting.addSetting(userLeaveMsgBeginIMSetting);
        userPaSetting.addSetting(userLeaveMsgEndIMSetting);
        userSetting.addSetting(userPaSetting);
        user.setSettings(userSetting);
    }

    private void setImNotificationSettings(User user, String setting1, String setting2,
                String setting3, String setting4) {
        user.setSettingValue("im_notification/conferenceEntryIM", setting1);
        user.setSettingValue("im_notification/conferenceExitIM", setting2);
        user.setSettingValue("im_notification/leaveMsgBeginIM", setting3);
        user.setSettingValue("im_notification/leaveMsgEndIM", setting4);
    }

}
