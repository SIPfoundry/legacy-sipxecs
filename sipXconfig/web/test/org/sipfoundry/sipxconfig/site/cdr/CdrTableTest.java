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
