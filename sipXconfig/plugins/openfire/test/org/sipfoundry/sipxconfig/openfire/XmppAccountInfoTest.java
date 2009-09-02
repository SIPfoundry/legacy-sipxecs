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

import static org.easymock.EasyMock.expectLastCall;

import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.StringReader;
import java.util.ArrayList;
import java.util.List;

import org.custommonkey.xmlunit.XMLTestCase;
import org.dom4j.Document;
import org.easymock.EasyMock;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.test.TestUtil;

public class XmppAccountInfoTest extends XMLTestCase {
    private List<User> m_users;

    @Override
    protected void setUp() throws Exception {

        User userOne = new User() {
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
        User userTwo = new User() {
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

        userOne.setUserName("One");
        userOne.setImId("One_IM");
        userOne.setImDisplayName("One_IM_DisplayName");
        userTwo.setUserName("Two");
        userTwo.setImDisplayName("Two_IM_DisplayName");
        userThree.setUserName("Three");
        userThree.setImId("Three_IM");

        m_users = new ArrayList<User>();
        m_users.add(userOne);
        m_users.add(userTwo);
        m_users.add(userThree);
    }

    public void testGenerate() throws Exception {
        CoreContext coreContext = EasyMock.createMock(CoreContext.class);
        coreContext.loadUsers();
        expectLastCall().andReturn(m_users).atLeastOnce();
        EasyMock.replay(coreContext);

        XmppAccountInfo xmppAccountInfo = new XmppAccountInfo();
        xmppAccountInfo.setCoreContext(coreContext);
        Document document = xmppAccountInfo.getDocument();
        String domDoc = TestUtil.asString(document);

        InputStream referenceXml = XmppAccountInfoTest.class.getResourceAsStream("xmpp-account-info.test.xml");
        assertXMLEqual(new InputStreamReader(referenceXml), new StringReader(domDoc));
    }
}
