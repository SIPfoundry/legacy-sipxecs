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
        supplyUserData();
        m_line.setUser(m_user);
    }

    public void testUserInfo() {
        assertEquals("jsmit", m_defaults.getAddress());
        assertEquals("jsmit@example.org", m_defaults.getScreenName());
        assertEquals("John Smit", m_defaults.getDisplayName());
        assertEquals("John Smit", m_defaults.getScreenName2());
        assertEquals("1234", m_defaults.getAuthorizationPassword());
        assertEquals("example.org", m_defaults.getRegistrationServer());
    }

    public void testEditUserInfo() {
        m_user.setUserName("500");
        assertEquals("500", m_defaults.getAddress());
        assertEquals("500@example.org", m_defaults.getScreenName());
        assertEquals("John Smit", m_defaults.getDisplayName());
        assertEquals("John Smit", m_defaults.getScreenName2());
        assertEquals("1234", m_defaults.getAuthorizationPassword());
        assertEquals("example.org", m_defaults.getRegistrationServer());
    }

    public void testResetUserInfo() {
        User m_newUser = new User();
        m_line.setUser(m_newUser);
        assertEquals("", m_defaults.getAddress());
        m_newUser.setUserName("400");
        assertEquals("400", m_defaults.getAddress());
        assertEquals("400@example.org", m_defaults.getScreenName());
        assertEquals(null, m_defaults.getDisplayName());
        assertEquals(null, m_defaults.getScreenName2());

        // Should Fix this : a default user password should be set in this case
        assertEquals(null, m_defaults.getAuthorizationPassword());

        assertEquals("example.org", m_defaults.getRegistrationServer());
    }

    private void supplyUserData() {
        m_user = new User();
        m_user.setUserName("jsmit");
        m_user.setFirstName("John");
        m_user.setLastName("Smit");
        m_user.setSipPassword("1234");
    }
}
