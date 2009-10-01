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
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import junit.framework.TestCase;
import org.apache.commons.io.IOUtils;
import org.dom4j.Document;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.conference.Bridge;
import org.sipfoundry.sipxconfig.conference.Conference;
import org.sipfoundry.sipxconfig.conference.ConferenceBridgeContext;
import org.sipfoundry.sipxconfig.permission.PermissionManagerImpl;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.type.SettingType;
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
    private Group m_group1;
    private Group m_group2;
    private Group m_group3;

    @Override
    protected void setUp() throws Exception {
        PermissionManagerImpl pm = new PermissionManagerImpl();
        pm.setModelFilesContext(TestHelper.getModelFilesContext(getModelDirectory("neoconf")));

        m_userOne = new User();
        m_userOne.setPermissionManager(pm);
        m_userOne.setSettingTypedValue("im/im-account", true);
        m_userOne.setSettingTypedValue("im/on-the-phone-message", "testing phone message");
        m_userOne.setSettingTypedValue("im/advertise-sip-presence", true);
        m_userOne.setSettingTypedValue("im/include-call-info", false);
        m_userOne.setUserName("One");
        m_userOne.setImId("One_IM");
        m_userOne.setImDisplayName("One_IM_DisplayName");

        m_userTwo = new User();
        m_userTwo.setPermissionManager(pm);
        m_userTwo.setSettingTypedValue("im/on-the-phone-message", "On the phone");
        m_userTwo.setSettingTypedValue("im/advertise-sip-presence", true);
        m_userTwo.setSettingTypedValue("im/include-call-info", false);
        m_userTwo.setUserName("Two");
        m_userTwo.setImDisplayName("Two_IM_DisplayName");

        User userThree = new User();
        userThree.setPermissionManager(pm);
        userThree.setSettingTypedValue("im/im-account", true);
        userThree.setSettingTypedValue("im/on-the-phone-message", "");
        userThree.setSettingTypedValue("im/advertise-sip-presence", true);
        userThree.setSettingTypedValue("im/include-call-info", false);
        userThree.setUserName("Three");

        m_users = new ArrayList<User>();
        m_users.addAll(Arrays.asList(m_userOne, m_userTwo, userThree));

        m_group1 = new Group() {
            @Override
            public Object getSettingTypedValue(SettingType type, String path) {
                if (path.equals("im/im-account")) {
                    return true;
                }
                return false;
            }
        };
        m_group1.setName("group1");
        m_group1.setDescription("my group");
        m_userOne.addGroup(m_group1);
        m_userTwo.addGroup(m_group1);

        m_group2 = new Group() {
            @Override
            public Object getSettingTypedValue(SettingType type, String path) {
                if (path.equals("im/im-account")) {
                    return true;
                }
                return false;
            }
        };
        m_group2.setName("group2");
        m_group2.setDescription("empty group");

        // the following group won't be replicated
        m_group3 = new Group();
        m_groups = Arrays.asList(m_group1, m_group2, m_group3);

        Bridge mockBridge = new Bridge() {
            @Override
            public String getHost() {
                return "servicename.domain.com";
            }
        };

        Conference conf1 = new Conference();
        conf1.setModelFilesContext(TestHelper.getModelFilesContext(getModelDirectory("neoconf")));
        conf1.setBridge(mockBridge);
        conf1.setSettingTypedValue("fs-conf-conference/participant-code", "123");
        conf1.setEnabled(true);
        conf1.setName("conf1");
        conf1.setDescription("Description");
        conf1.setExtension("300");
        conf1.setSettingTypedValue("chat-meeting/moderated", true);
        conf1.setSettingTypedValue("chat-meeting/log-conversations", false);
        conf1.setOwner(m_userOne);

        Conference conf2 = new Conference();

        Conference conf3 = new Conference();
        conf3.setModelFilesContext(TestHelper.getModelFilesContext(getModelDirectory("neoconf")));
        conf3.setBridge(mockBridge);
        conf3.setEnabled(true);
        conf3.setName("conf3");
        conf3.setDescription("Description 3");
        conf3.setExtension("300");
        conf3.setSettingTypedValue("chat-meeting/moderated", false);
        conf3.setSettingTypedValue("chat-meeting/log-conversations", true);
        conf3.setOwner(userThree);

        m_conferences = Arrays.asList(conf1, conf2, conf3);

    }

    public void testGenerate() throws Exception {
        CoreContext coreContext = createMock(CoreContext.class);

        coreContext.loadUsers();
        expectLastCall().andReturn(m_users).atLeastOnce();
        coreContext.getGroups();
        expectLastCall().andReturn(m_groups);
        coreContext.getGroupMembers(m_group1);
        expectLastCall().andReturn(Arrays.asList(m_userOne, m_userTwo)).once();
        coreContext.getGroupMembers(m_group2);
        expectLastCall().andReturn(null).once();
        replay(coreContext);

        ConferenceBridgeContext m_conferenceContext = createMock(ConferenceBridgeContext.class);

        m_conferenceContext.getAllConferences();
        expectLastCall().andReturn(m_conferences).atLeastOnce();
        replay(m_conferenceContext);

        XmppAccountInfo xmppAccountInfo = new XmppAccountInfo();
        xmppAccountInfo.setCoreContext(coreContext);
        xmppAccountInfo.setConferenceBridgeContext(m_conferenceContext);

        Document document = xmppAccountInfo.getDocument();
        String domDoc = TestUtil.asString(document);

        InputStream referenceXml = XmppAccountInfoTest.class.getResourceAsStream("xmpp-account-info.test.xml");
        assertEquals(IOUtils.toString(referenceXml), domDoc);
    }
}
