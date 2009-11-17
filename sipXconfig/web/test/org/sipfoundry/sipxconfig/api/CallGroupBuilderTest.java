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

import java.util.ArrayList;
import java.util.List;

import junit.framework.TestCase;

import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.sipfoundry.sipxconfig.common.CoreContext;

public class CallGroupBuilderTest extends TestCase {
    private static final String USER_NAME = "joe";
    private static final org.sipfoundry.sipxconfig.common.User USER =
        new org.sipfoundry.sipxconfig.common.User();
    static {
        USER.setUserName(USER_NAME);
    }
    private CallGroupBuilder m_builder;
    private org.sipfoundry.sipxconfig.admin.callgroup.CallGroup m_myCallGroup;
    private CallGroup m_apiCallGroup;
    private IMocksControl m_control;
    private CoreContext m_coreContext;

    @Override
    protected void setUp() {
        m_control = EasyMock.createControl();
        m_coreContext = m_control.createMock(CoreContext.class);
        m_builder = new CallGroupBuilder();
        m_builder.setCoreContext(m_coreContext);
        m_myCallGroup = new org.sipfoundry.sipxconfig.admin.callgroup.CallGroup();
        m_apiCallGroup = new CallGroup();
    }

    public void testFromApi() {
        m_apiCallGroup.setName("cg1");
        m_apiCallGroup.setExtension("101");
        m_apiCallGroup.setDescription("i'm still alive");
        m_apiCallGroup.setEnabled(new Boolean(true));

        // Create a test UserRing
        UserRing apiRing = new UserRing();
        apiRing.setExpiration(19);
        apiRing.setPosition(0);     // must be zero, this is its position in the list
        apiRing.setType(AbstractRingBuilder.TYPE_IMMEDIATE);
        apiRing.setUserName(USER_NAME);
        UserRing[] apiRings = new UserRing[1];
        apiRings[0] = apiRing;
        m_apiCallGroup.setRings(apiRings);

        // set up the mock core context
        m_coreContext.loadUserByUserName(USER_NAME);
        m_control.andReturn(USER);
        m_control.replay();

        ApiBeanUtil.toMyObject(m_builder, m_myCallGroup, m_apiCallGroup);
        assertEquals("cg1", m_myCallGroup.getName());
        assertEquals("101", m_myCallGroup.getExtension());
        assertEquals("i'm still alive", m_myCallGroup.getDescription());
        assertEquals(true, m_myCallGroup.isEnabled());
        List myRings = m_myCallGroup.getRings();
        assertEquals(1, myRings.size());
        org.sipfoundry.sipxconfig.admin.callgroup.UserRing myRing =
            (org.sipfoundry.sipxconfig.admin.callgroup.UserRing) myRings.get(0);
        assertEquals(19, myRing.getExpiration());
        assertEquals(org.sipfoundry.sipxconfig.admin.callgroup.AbstractRing.Type.IMMEDIATE, myRing.getType());
    }

    public void testToApi() {
        m_myCallGroup.setName("cg1");
        m_myCallGroup.setExtension("101");
        m_myCallGroup.setDescription("i'm still alive");
        m_myCallGroup.setEnabled(true);

        // Create a test UserRing
        org.sipfoundry.sipxconfig.admin.callgroup.UserRing myRing =
            new org.sipfoundry.sipxconfig.admin.callgroup.UserRing();
        myRing.setCallGroup(m_myCallGroup);
        myRing.setExpiration(19);
        myRing.setType(org.sipfoundry.sipxconfig.admin.callgroup.AbstractRing.Type.IMMEDIATE);
        myRing.setUser(USER);
        List myRings = new ArrayList(1);
        myRings.add(myRing);
        m_myCallGroup.setRings(myRings);

        ApiBeanUtil.toApiObject(m_builder, m_apiCallGroup, m_myCallGroup);
        assertEquals("cg1", m_apiCallGroup.getName());
        assertEquals("101", m_apiCallGroup.getExtension());
        assertEquals("i'm still alive", m_apiCallGroup.getDescription());
        assertEquals(true, m_apiCallGroup.getEnabled().booleanValue());
        UserRing[] apiRings = m_apiCallGroup.getRings();
        assertEquals(1, apiRings.length);
        UserRing apiRing = apiRings[0];
        assertEquals(19, apiRing.getExpiration());
        assertEquals(0, apiRing.getPosition());
        assertEquals(AbstractRingBuilder.TYPE_IMMEDIATE, apiRing.getType());
    }
}
