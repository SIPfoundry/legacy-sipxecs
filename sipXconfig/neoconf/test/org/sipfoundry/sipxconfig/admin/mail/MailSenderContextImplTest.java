/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.admin.mail;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.domain.Domain;
import org.sipfoundry.sipxconfig.domain.DomainManager;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;

public class MailSenderContextImplTest extends TestCase {

    private DomainManager m_domainManager;

    protected void setUp() throws Exception {
        Domain domain = new Domain();
        domain.setName("example.com");

        m_domainManager = createMock(DomainManager.class);
        m_domainManager.getDomain();
        expectLastCall().andReturn(domain);
        replay(m_domainManager);
    }

    protected void tearDown() throws Exception {
        verify(m_domainManager);
    }

    public void testGetFullDomain() {
        MailSenderContextImpl ms = new MailSenderContextImpl();
        ms.setDomainManager(m_domainManager);
        assertEquals("user@example.com", ms.getFullAddress("user"));
        assertEquals("user@example.org", ms.getFullAddress("user@example.org"));
    }
}
