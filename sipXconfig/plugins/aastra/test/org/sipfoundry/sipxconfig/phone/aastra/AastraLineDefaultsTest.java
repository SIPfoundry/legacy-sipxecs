/*
 * 
 * 
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxconfig.phone.aastra;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.device.DeviceDefaults;
import org.sipfoundry.sipxconfig.phone.Line;

public class AastraLineDefaultsTest extends TestCase {
    private AastraLineDefaults m_defaults;
    private Line m_line;
    private User m_user;

    protected void setUp() {
        AastraPhone phone = new AastraPhone();
        m_line = phone.createLine();
        DeviceDefaults defaults = new DeviceDefaults();
        defaults.setDomainManager(TestHelper.getTestDomainManager("example.org"));
        m_defaults = new AastraLineDefaults(defaults, m_line);
        m_user = new User();
        m_user.setUserName("shruthi");
    }

    public void testUserInfo() {
        assertNull(m_defaults.getDisplayName());
        m_line.setUser(m_user);
        assertEquals("shruthi", m_defaults.getAddress());
    }
}
