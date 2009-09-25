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
import org.sipfoundry.sipxconfig.setting.Group;
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

        m_userOne = new User() {
            @Override
            public Object getSettingTypedValue(String path) {
                Object ret = null;
                if (path.equals("openfire/on-the-phone-message")) {
                    ret = "testing phone message";
                } else if (path.equals("openfire/advertise-sip-presence")) {
                    ret = true;
                } else if (path.equals("openfire/include-call-info")) {
                    ret = false;
                }
                return ret;
            }
        };
        m_userTwo = new User() {
            @Override
            public Object getSettingTypedValue(String path) {
                Object ret = null;
                if (path.equals("openfire/on-the-phone-message")) {
                    ret = "On the phone";
                } else if (path.equals("openfire/advertise-sip-presence")) {
                    ret = true;
                } else if (path.equals("openfire/include-call-info")) {
                    ret = false;
                }
                return ret;
            }
        };
        User userThree = new User() {
            @Override
            public Object getSettingTypedValue(String path) {
                Object ret = null;
                if (path.equals("openfire/on-the-phone-message")) {
                    ret = "";
                } else if (path.equals("openfire/advertise-sip-presence")) {
                    ret = true;
                } else if (path.equals("openfire/include-call-info")) {
                    ret = false;
                }
                return ret;
            }
        };

        m_userOne.setUserName("One");
        m_userOne.setImId("One_IM");
        m_userOne.setImDisplayName("One_IM_DisplayName");
        m_userTwo.setUserName("Two");
        m_userTwo.setImDisplayName("Two_IM_DisplayName");
        userThree.setUserName("Three");
        userThree.setImId("Three_IM");

        m_users = new ArrayList<User>();
        m_users.add(m_userOne);
        m_users.add(m_userTwo);
        m_users.add(userThree);

        m_group1 = new Group() {
            @Override
            public String getSettingValue(String path) {
                if (path.equals("openfire/replicate-group")) {
                    return "1";
                }
                return null;
            }
        };
        m_group1.setName("group1");
        m_group1.setDescription("my group");
        m_userOne.addGroup(m_group1);
        m_userTwo.addGroup(m_group1);

        m_group2 = new Group() {
            @Override
            public String getSettingValue(String path) {
                if (path.equals("openfire/replicate-group")) {
                    return "1";
                }
                return null;
            }
        };
        m_group2.setName("group2");
        m_group2.setDescription("empty group");

        // the following group won't be replicated
        m_group3 = new Group();

        m_groups = new ArrayList<Group>();
        m_groups.add(m_group1);
        m_groups.add(m_group2);
        m_groups.add(m_group3);

        m_conferences = new ArrayList<Conference>();

        Conference conf1 = new Conference() {
            @Override
            public Bridge getBridge() {
                Bridge mockBridge = new Bridge() {
                    @Override
                    public String getHost() {
                        return "servicename.domain.com";
                    }
                };
                return mockBridge;
            }

            @Override
            public String getParticipantAccessCode() {
                return "123";
            }
        };
        conf1.setModelFilesContext(TestHelper.getModelFilesContext(getModelDirectory("neoconf")));
        conf1.setEnabled(true);
        conf1.setName("conf1");
        conf1.setDescription("Description");
        conf1.setExtension("300");
        conf1.setSettingTypedValue("chat-meeting/moderated", true);
        conf1.setSettingTypedValue("chat-meeting/log-conversations", false);
        conf1.setOwner(m_userOne);
        m_conferences.add(conf1);

        Conference conf2 = new Conference();
        m_conferences.add(conf2);
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
