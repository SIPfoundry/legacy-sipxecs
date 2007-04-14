/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.conference;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.TestHelper;
import org.springframework.context.ApplicationContext;

public class ConferenceBeanTest extends TestCase {
    private ApplicationContext m_applicationContext;

    protected void setUp() throws Exception {
        m_applicationContext = TestHelper.getApplicationContext();
    }

    public void testConference() throws Exception {
        Conference conference = (Conference) m_applicationContext.getBean(Conference.BEAN_NAME,
                Conference.class);
        assertNotNull(conference.getSettings());
    }

    public void testBridge() throws Exception {
        Bridge bridge = (Bridge) m_applicationContext.getBean(Bridge.BEAN_NAME, Bridge.class);
        assertNotNull(bridge.getSettings());
    }
}
