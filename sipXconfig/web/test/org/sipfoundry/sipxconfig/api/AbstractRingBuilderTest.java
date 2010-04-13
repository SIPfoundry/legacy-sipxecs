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

public class AbstractRingBuilderTest extends TestCase {
    private AbstractRingBuilder m_builder;
    private org.sipfoundry.sipxconfig.admin.callgroup.AbstractRing m_myAbstractRing;
    private UserRing m_apiAbstractRing;

    protected void setUp() {
        m_builder = new AbstractRingBuilder();

        // UserRing copies over user name
        m_builder.getCustomFields().add("userName");

        m_myAbstractRing = new org.sipfoundry.sipxconfig.admin.callgroup.UserRing();
        m_apiAbstractRing = new UserRing();
    }

    public void testFromApi() {
        m_apiAbstractRing.setType(AbstractRingBuilder.TYPE_IMMEDIATE);
        ApiBeanUtil.toMyObject(m_builder, m_myAbstractRing, m_apiAbstractRing);
        assertEquals(org.sipfoundry.sipxconfig.admin.callgroup.AbstractRing.Type.IMMEDIATE,
                m_myAbstractRing.getType());
    }

    public void testToApi() {
        m_myAbstractRing.setType(org.sipfoundry.sipxconfig.admin.callgroup.AbstractRing.Type.IMMEDIATE);
        ApiBeanUtil.toApiObject(m_builder, m_apiAbstractRing, m_myAbstractRing);
        assertEquals(AbstractRingBuilder.TYPE_IMMEDIATE, m_apiAbstractRing.getType());
    }
}
