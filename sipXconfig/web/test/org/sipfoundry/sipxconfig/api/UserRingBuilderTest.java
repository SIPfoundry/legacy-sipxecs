/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.api;

import junit.framework.TestCase;

import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.sipfoundry.sipxconfig.common.CoreContext;

public class UserRingBuilderTest extends TestCase {
    private static final String USER_NAME = "joe";
    private static final org.sipfoundry.sipxconfig.common.User USER =
        new org.sipfoundry.sipxconfig.common.User();
    static {
        USER.setUserName(USER_NAME);
    }

    private UserRingBuilder m_builder;
    private org.sipfoundry.sipxconfig.admin.callgroup.UserRing m_myUserRing;
    private UserRing m_apiUserRing;
    private IMocksControl m_control;
    private CoreContext m_coreContext;

    protected void setUp() {
        m_control = EasyMock.createControl();
        m_coreContext = m_control.createMock(CoreContext.class);
        m_builder = new UserRingBuilder(m_coreContext);
        m_myUserRing = new org.sipfoundry.sipxconfig.admin.callgroup.UserRing();
        m_apiUserRing = new UserRing();
        m_apiUserRing.setType(AbstractRingBuilder.TYPE_IMMEDIATE);
    }

    public void testFromApi() {
        // set up the mock core context
        m_coreContext.loadUserByUserName(USER_NAME);
        m_control.andReturn(USER);
        m_control.replay();

        m_apiUserRing.setUserName(USER_NAME);
        ApiBeanUtil.toMyObject(m_builder, m_myUserRing, m_apiUserRing);
        assertEquals(USER, m_myUserRing.getUser());
        m_control.verify();
    }

    public void testToApi() {
        m_myUserRing.setUser(USER);
        ApiBeanUtil.toApiObject(m_builder, m_apiUserRing, m_myUserRing);
        assertEquals(USER_NAME, m_apiUserRing.getUserName());
    }
}
