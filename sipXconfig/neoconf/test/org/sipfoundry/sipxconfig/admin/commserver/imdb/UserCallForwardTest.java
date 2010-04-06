/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.admin.commserver.imdb;

import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.StringReader;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Map;

import org.custommonkey.xmlunit.XMLTestCase;
import org.custommonkey.xmlunit.XMLUnit;
import org.dom4j.Document;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.XmlUnitHelper;
import org.sipfoundry.sipxconfig.admin.forwarding.CallSequence;
import org.sipfoundry.sipxconfig.admin.forwarding.ForwardingContext;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.DaoUtils;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.permission.PermissionManagerImpl;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;

public class UserCallForwardTest extends XMLTestCase {
    public UserCallForwardTest() {
        XmlUnitHelper.setNamespaceAware(false);
        XMLUnit.setIgnoreWhitespace(true);
    }

    private final String[][] USER_DATA = {
        {
            "first1", "last1", "mir1", "20"
        }, {
            "first2", "last2", "mir2", "8"
        }, {
            "first3", "last3", "mir3", "192"
        },
    };

    private List<User> m_users;
    private List<CallSequence> m_cs;

    @Override
    protected void setUp() throws Exception {
        PermissionManagerImpl impl = new PermissionManagerImpl();
        impl.setModelFilesContext(TestHelper.getModelFilesContext());

        m_users = new ArrayList<User>();
        m_cs = new ArrayList<CallSequence>();
        for (String[] ud : USER_DATA) {
            User user = new User();
            user.setPermissionManager(impl);

            user.setFirstName(ud[0]);
            user.setLastName(ud[1]);
            user.setUserName(ud[2]);
            m_users.add(user);

            CallSequence cs = new CallSequence();
            cs.setUser(user);
            cs.setCfwdTime(Integer.valueOf(ud[3]));

            m_cs.add(cs);
        }
    }

    public void testGenerateEmpty() throws Exception {
        UserForward uf = new UserForward() {
            @Override
            protected String getSipDomain() {
                return "example.org";
            }
        };

        CoreContext coreContext = createMock(CoreContext.class);
        coreContext.loadUsersByPage(0, DaoUtils.PAGE_SIZE);
        expectLastCall().andReturn(Collections.emptyList());

        uf.setCoreContext(coreContext);

        replay(coreContext);

        List<Map<String, String>> document = uf.generate();
        assertEquals(0, document.size());

        verify(coreContext);
    }

    public void testGenerate() throws Exception {
        UserForward uf = new UserForward() {
            @Override
            protected String getSipDomain() {
                return "example.org";
            }
        };

        CoreContext coreContext = createMock(CoreContext.class);
        coreContext.loadUsersByPage(0, DaoUtils.PAGE_SIZE);
        expectLastCall().andReturn(m_users);
        ForwardingContext forwardingContext = createMock(ForwardingContext.class);
        for (int i = 0; i < m_users.size(); i++) {
            forwardingContext.getCallSequenceForUser(m_users.get(i));
            expectLastCall().andReturn(m_cs.get(i));
        }

        uf.setCoreContext(coreContext);
        uf.setForwardingContext(forwardingContext);

        replay(coreContext, forwardingContext);

        Document document = uf.generateXml();
        String ufXml = XmlUnitHelper.asString(document);

        InputStream referenceXmlStream = UserCallForwardTest.class.getResourceAsStream("userforward.test.xml");
        assertXMLEqual(new InputStreamReader(referenceXmlStream), new StringReader(ufXml));

        verify(coreContext, forwardingContext);
    }
}
