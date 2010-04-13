/*
 *
 *
 * Copyright (C) 2010 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.cdr;

import static org.sipfoundry.sipxconfig.security.UserRole.Admin;
import static org.sipfoundry.sipxconfig.security.UserRole.User;

import java.util.HashMap;
import java.util.Map;

import junit.framework.TestCase;

import org.apache.commons.lang.StringUtils;
import org.apache.tapestry.test.Creator;
import org.easymock.EasyMock;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.SipUri;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.domain.Domain;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.security.UserDetailsImpl;
import org.sipfoundry.sipxconfig.sip.SipServiceImpl;
import org.sipfoundry.sipxconfig.site.UserSession;

public class CdrTableTest extends TestCase {
    private CdrTable m_cdrTable;

    public void setUp() {
        //domain mock
        Domain domain = new Domain();
        domain.setName("testa.testb");
        DomainManager dm = EasyMock.createMock(DomainManager.class);
        dm.getDomain();
        EasyMock.expectLastCall().andReturn(domain);
        EasyMock.replay(dm);

        //userSession mock
        UserSession sessMock = new MockUserSession(false);

        Creator pageCreator = new Creator();
        //properties map used to instantiate the CDR table.
        Map<String, Object> propertiesMap = new HashMap<String, Object>();
        propertiesMap.put("sipService", new SipServiceMock());
        propertiesMap.put("domainManager", dm);
        propertiesMap.put("userSession", sessMock);
        m_cdrTable = (CdrTable) pageCreator.newInstance(CdrTable.class, propertiesMap);
    }

    public void testFullNameCall() {
        m_cdrTable.call("Frank Zappa - 705");
    }

    public void testOneNameCall() {
        m_cdrTable.call("Frank - 5309");
    }

    public void testNoNameCall() {
        m_cdrTable.call("55555");
    }

    public void testSelfCallCondition () {
        String caller1 = "\"James Bond\"<sip:200@domain.com>";
        String user1 = "<sip:200@domain.com>";
        assertTrue(m_cdrTable.isSelfCallCondition(caller1, user1));

        String caller2 = "\"201\" <sip:201@domain.com>";
        String user2 = "sip:201@domain.com";
        assertTrue(m_cdrTable.isSelfCallCondition(caller2, user2));

        String caller3 = "<sip:202@11.12.13.14>";
        String user3 = "\"Yoggy Bear\" <sip:202@11.12.13.14>";
        assertTrue(m_cdrTable.isSelfCallCondition(caller3, user3));

        String caller4 = "<sip:203@domain.com>";
        String user4 = "sip:203@domain.com";
        assertTrue(m_cdrTable.isSelfCallCondition(caller4, user4));

        String caller5 = "\"Big Whale\" <sip:205@domain.com>";
        String user5 = "\"Bigger Whale\" <sip:205@domain.com>";
        assertTrue(m_cdrTable.isSelfCallCondition(caller5, user5));

        String caller6 = "<sip:204@domain>";
        String user6 = "sip:204@domain";
        assertFalse(m_cdrTable.isSelfCallCondition(caller6, user6));

        String caller7 = "<sip:207@domain.com>";
        String user7 = "<sip:207@domain.something.com>";
        assertFalse(m_cdrTable.isSelfCallCondition(caller7, user7));
    }

    //userSession mock class
    private static class MockUserSession extends UserSession {
        private final UserDetailsImpl m_userDetailsImpl;

        MockUserSession(boolean admin) {
            User user = new User();
            if (admin) {
                m_userDetailsImpl = new UserDetailsImpl(user, "bongo", User.toAuth(), Admin.toAuth());
            } else {
                m_userDetailsImpl = new UserDetailsImpl(user, "bongo", User.toAuth());
            }
        }

        @Override
        protected UserDetailsImpl getUserDetails() {
            return m_userDetailsImpl;
        }

        public User getUser(CoreContext coreContext) {
            return new User();
        }
    }

    //sipService mock class
    private class SipServiceMock extends SipServiceImpl {
        public void sendRefer(User user, String sourceAddrSpec, String displayName, String destinationAddrSpec) {
            String userAsString = SipUri.extractUser(destinationAddrSpec);
            assertTrue(StringUtils.isNumeric(userAsString));
        }
    }
}
