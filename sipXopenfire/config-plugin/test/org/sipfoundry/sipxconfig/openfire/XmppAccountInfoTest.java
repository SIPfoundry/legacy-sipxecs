/*
 *
 *
 * Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.openfire;

import java.io.InputStream;
import java.util.Arrays;
import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.List;

import static java.util.Arrays.asList;
import static java.util.Collections.emptyList;

import junit.framework.TestCase;
import org.apache.commons.io.IOUtils;
import org.dom4j.Document;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.DaoUtils;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.conference.Bridge;
import org.sipfoundry.sipxconfig.conference.Conference;
import org.sipfoundry.sipxconfig.conference.ConferenceBridgeContext;
import org.sipfoundry.sipxconfig.permission.PermissionManagerImpl;
import org.sipfoundry.sipxconfig.service.SipxImbotService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.ModelFilesContext;
import org.sipfoundry.sipxconfig.test.TestUtil;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.sipfoundry.sipxconfig.test.TestUtil.getModelDirectory;

public class XmppAccountInfoTest extends TestCase {
    private List<User> m_users;
    private List<Group> m_groups;
    private List<Conference> m_conferences;
    private User m_userOne;
    private User m_userTwo;
    private User m_userThree;
    private User m_userFour;
    private User m_userFive;
    private Group m_group1;
    private Group m_group2;
    private Group m_group3;
    private Group m_group4;
    private ModelFilesContext m_neoconfModelFilesContext;
    private PermissionManagerImpl m_pm;
    private LocationsManager m_locationsManager;
    private SipxServiceManager m_sipxServiceManager;
    private SipxImbotService m_sipxImbotService = new SipxImbotService();


    @Override
    protected void setUp() throws Exception {
        m_neoconfModelFilesContext = TestHelper.getModelFilesContext(getModelDirectory("neoconf"));
        Location location = new Location();
        location.setAddress("192.168.1.2");
        location.setFqdn("puppy.org");

        m_sipxServiceManager = createMock(SipxServiceManager.class);
        m_locationsManager = createMock(LocationsManager.class);
        m_locationsManager.getLocationsForService(m_sipxImbotService);
        expectLastCall().andReturn(Arrays.asList(location)).anyTimes();

        replay(m_locationsManager);

        m_sipxImbotService.setLocationsManager(m_locationsManager);

        m_pm = new PermissionManagerImpl();
        m_pm.setModelFilesContext(m_neoconfModelFilesContext);

        m_group1 = new ImGroup(true, true, true);
        m_group1.setName("group1");
        m_group1.setDescription("my group");

        // m_group2 im-group is enabled, im-account is disabled
        m_group2 = new ImGroup(true, false, true);
        m_group2.setName("group2");
        m_group2.setDescription("empty IM group");

        // the following group won't be replicated
        m_group3 = new Group();

        // m_group4 im-group is disabled, im-account is enabled
        m_group4 = new ImGroup(false, true, false);
        m_group4.setName("group4");
        m_group4.setDescription("NO IM group");

        m_groups = asList(m_group1, m_group2, m_group3, m_group4);

        m_userOne = new User();
        m_userOne.setPermissionManager(m_pm);
        m_userOne.addGroup(m_group1);
        m_userOne.addGroup(m_group3);
        m_userOne.setSettingTypedValue("im/on-the-phone-message", "testing phone message");
        m_userOne.setSettingTypedValue("im/advertise-sip-presence", true);
        m_userOne.setUserName("One");
        m_userOne.setImId("One_IM");
        m_userOne.setImDisplayName("One_IM_DisplayName");
        m_userOne.setEmailAddress("one@puppy.org");

        m_userTwo = new User();
        m_userTwo.setPermissionManager(m_pm);
        m_userTwo.addGroup(m_group2);
        m_userTwo.setUserName("Two");
        m_userTwo.setImDisplayName("Two_IM_DisplayName");
        m_userTwo.setEmailAddress("two@puppy.org");

        m_userThree = new User();
        m_userThree.setPermissionManager(m_pm);
        m_userThree.setSettingTypedValue("im/im-account", true);
        m_userThree.setSettingTypedValue("im/on-the-phone-message", "");
        m_userThree.setSettingTypedValue("im/advertise-sip-presence", true);
        m_userThree.setSettingTypedValue("im/include-call-info", true);
        m_userThree.setUserName("Three");
        m_userThree.setImPassword("bongoImPassword");
        m_userThree.setEmailAddress("three@puppy.org");

        m_userFour = new User();
        m_userFour.setPermissionManager(m_pm);
        m_userFour.addGroup(m_group1);
        m_userFour.addGroup(m_group4);
        m_userFour.setUserName("Four");
        m_userFour.setImDisplayName("Four_IM_DisplayName");

        m_userFive = new User();
        m_userFive.setPermissionManager(m_pm);
        m_userFive.addGroup(m_group4);
        m_userFive.setUserName("Five");
        m_userFive.setFirstName("Five_DisplayName");

        m_users = new ArrayList<User>();
        m_users.addAll(asList(m_userOne, m_userTwo, m_userThree, m_userFour, m_userFive));

        Bridge mockBridge = new Bridge() {
            @Override
            public String getHost() {
                return "servicename.domain.com";
            }
        };

        Conference conf1 = new Conference();
        conf1.setModelFilesContext(m_neoconfModelFilesContext);
        conf1.setBridge(mockBridge);
        conf1.setSettingTypedValue("fs-conf-conference/participant-code", "123");
        conf1.setEnabled(true);
        conf1.setName("conf1");
        conf1.setDescription("Description");
        conf1.setExtension("300");
        conf1.setSettingTypedValue("chat-meeting/moderated", true);
        conf1.setOwner(m_userOne);

        Conference conf2 = new Conference();

        Conference conf3 = new Conference();
        conf3.setModelFilesContext(m_neoconfModelFilesContext);
        conf3.setBridge(mockBridge);
        conf3.setEnabled(true);
        conf3.setName("conf3");
        conf3.setDescription("Description 3");
        conf3.setExtension("300");
        conf3.setSettingTypedValue("chat-meeting/moderated", false);
        conf3.setOwner(m_userThree);

        m_conferences = asList(conf1, conf2, conf3);

    }

    public void testGenerate() throws Exception {
        CoreContext coreContext = createMock(CoreContext.class);

        coreContext.loadUsersByPage(0, DaoUtils.PAGE_SIZE);
        expectLastCall().andReturn(m_users).atLeastOnce();
        coreContext.getGroups();
        expectLastCall().andReturn(m_groups);
        coreContext.getGroupMembers(m_group1);
        expectLastCall().andReturn(asList(m_userOne, m_userTwo, m_userFour)).once();
        coreContext.getGroupMembers(m_group2);
        expectLastCall().andReturn(asList(m_userTwo)).once();
        coreContext.getGroupMembers(m_group3);
        expectLastCall().andReturn(emptyList()).once();
        coreContext.getGroupMembers(m_group4);
        expectLastCall().andReturn(asList(m_userFour, m_userFive)).once();
        User paUser = new User();
        paUser.setPermissionManager(m_pm);
        coreContext.newUser();
        expectLastCall().andReturn(paUser);
        replay(coreContext);

        ConferenceBridgeContext m_conferenceContext = createMock(ConferenceBridgeContext.class);

        m_conferenceContext.getAllConferences();
        expectLastCall().andReturn(m_conferences).atLeastOnce();
        replay(m_conferenceContext);

        m_sipxImbotService.setBeanName(SipxImbotService.BEAN_ID);
        m_sipxImbotService.setModelDir("sipximbot");
        m_sipxImbotService.setModelName("sipximbot.xml");
        m_sipxImbotService.setModelFilesContext(m_neoconfModelFilesContext);
        m_sipxImbotService.setSettingValue("imbot/imId", "MyAssistant");
        m_sipxImbotService.setSettingValue("imbot/imPassword", "password");

        SipxServiceManager m_sipxServiceManager = createMock(SipxServiceManager.class);
        m_sipxServiceManager.getServiceByBeanId(SipxImbotService.BEAN_ID);
        expectLastCall().andReturn(m_sipxImbotService).atLeastOnce();
        replay(m_sipxServiceManager);

        paUser.setSipxImbotService(m_sipxImbotService);

        XmppAccountInfo xmppAccountInfo = new XmppAccountInfo();
        xmppAccountInfo.setCoreContext(coreContext);
        xmppAccountInfo.setConferenceBridgeContext(m_conferenceContext);
        xmppAccountInfo.setSipxServiceManager(m_sipxServiceManager);

        Document document = xmppAccountInfo.getDocument();
        String domDoc = TestUtil.asString(document);

        InputStream referenceXml = XmppAccountInfoTest.class.getResourceAsStream("xmpp-account-info.test.xml");
        assertEquals(IOUtils.toString(referenceXml), domDoc);
    }

    private class ImGroup extends Group {
        public ImGroup(boolean imGroup, boolean imAccount, boolean addPaToGroup) {
            HashMap<String, String> values = new HashMap<String, String>();
            if (imGroup) {
                values.put("im/im-group", "1");
                if (addPaToGroup) {
                    values.put("im/add-pa-to-group", "1");
                }
            }
            if (imAccount) {
                values.put("im/im-account", "1");
            }
            setDatabaseValues(values);
        }
    }
}
