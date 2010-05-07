/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.site.user_portal;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.isA;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;
import junit.framework.TestCase;

import org.apache.hivemind.Messages;
import org.apache.hivemind.util.PropertyUtils;
import org.apache.tapestry.test.Creator;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.domain.Domain;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.sip.SipService;
import org.sipfoundry.sipxconfig.test.PhonebookTestHelper;

public class UserPhonebookPageTest extends TestCase {

    private static final int PORTAL_USER_ID = 1;
    private Creator m_pageCreator;
    private User m_portalUser;
    private CoreContext m_coreContext;
    private UserPhonebookPage m_out;
    private PhonebookTestHelper m_testHelper;

    @Override
    public void setUp() {
        m_portalUser = new User();
        m_portalUser.setName("portalUser");
        m_testHelper = new PhonebookTestHelper();

        m_coreContext = createMock(CoreContext.class);
        m_testHelper.configureCoreContextMock(m_coreContext);

        m_coreContext.loadUser(PORTAL_USER_ID);
        expectLastCall().andReturn(m_portalUser).anyTimes();

        m_pageCreator = new Creator();

        m_out = (UserPhonebookPage) m_pageCreator.newInstance(UserPhonebookPage.class);

        Messages messages = createMock(Messages.class);
        messages.getMessage(isA(String.class));
        expectLastCall().andReturn("").anyTimes();

        m_out.setUserId(1);

        Domain domain = new Domain();
        domain.setName("example.com");
        DomainManager domainManager = createMock(DomainManager.class);
        domainManager.getDomain();
        expectLastCall().andReturn(domain).anyTimes();

        PropertyUtils.write(m_out, "coreContext", m_coreContext);
        PropertyUtils.write(m_out, "messages", messages);
        PropertyUtils.write(m_out, "domainManager", domainManager);

        replay(m_coreContext, messages, domainManager);
    }

    public void testCall() {
        SipService sipService = createMock(SipService.class);
        sipService.sendRefer(m_portalUser, "sip:portalUser@example.com", "ClickToCall", "sip:123@example.com");
        sipService.sendRefer(m_portalUser, "sip:portalUser@example.com", "ClickToCall", "sip:123@example.org");
        replay(sipService);

        PropertyUtils.write(m_out, "sipService", sipService);

        m_out.call("123");
        m_out.call("123@example.org");

        verify(sipService);
    }
}
