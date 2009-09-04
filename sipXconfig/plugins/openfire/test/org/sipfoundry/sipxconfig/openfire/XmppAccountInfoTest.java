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
import java.io.InputStreamReader;
import java.io.StringReader;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import org.custommonkey.xmlunit.XMLTestCase;
import org.dom4j.Document;
import org.easymock.EasyMock;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.test.TestUtil;

import static org.easymock.EasyMock.expectLastCall;

public class XmppAccountInfoTest extends XMLTestCase {
    private List<User> m_users;
    private List<Group> m_groups;
    private User m_userOne;
    private User m_userTwo;
    private Group m_group1;
    private Group m_group2;

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

        m_group1 = new Group();
        m_group1.setName("group1");
        m_group1.setDescription("my group");
        m_userOne.addGroup(m_group1);
        m_userTwo.addGroup(m_group1);

        m_group2 = new Group();
        m_group2.setName("group2");
        m_group2.setDescription("empty group");

        m_groups = new ArrayList<Group>();
        m_groups.add(m_group1);
        m_groups.add(m_group2);
    }

    public void testGenerate() throws Exception {
        CoreContext coreContext = EasyMock.createMock(CoreContext.class);
        coreContext.loadUsers();
        expectLastCall().andReturn(m_users).atLeastOnce();
        coreContext.getGroups();
        expectLastCall().andReturn(m_groups);
        coreContext.getGroupMembers(m_group1);
        expectLastCall().andReturn(Arrays.asList(m_userOne, m_userTwo)).once();
        coreContext.getGroupMembers(m_group2);
        expectLastCall().andReturn(null).once();
        EasyMock.replay(coreContext);

        XmppAccountInfo xmppAccountInfo = new XmppAccountInfo();
        xmppAccountInfo.setCoreContext(coreContext);
        Document document = xmppAccountInfo.getDocument();
        String domDoc = TestUtil.asString(document);

        InputStream referenceXml = XmppAccountInfoTest.class.getResourceAsStream("xmpp-account-info.test.xml");
        assertXMLEqual(new InputStreamReader(referenceXml), new StringReader(domDoc));
    }
}
