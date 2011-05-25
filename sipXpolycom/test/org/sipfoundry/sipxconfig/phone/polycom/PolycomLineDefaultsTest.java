/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.phone.polycom;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.device.DeviceDefaults;
import org.sipfoundry.sipxconfig.phone.Line;

public class PolycomLineDefaultsTest extends TestCase {
    private PolycomLineDefaults m_defaults;
    private Line m_line;
    private User m_user;

    @Override
    protected void setUp() {
        PolycomPhone phone = new PolycomPhone();
        m_line = phone.createLine();
        DeviceDefaults defaults = new DeviceDefaults();
        defaults.setDomainManager(TestHelper.getTestDomainManager("example.org"));
        m_defaults = new PolycomLineDefaults(defaults, m_line);
        m_user = new User();
        m_user.setUserName("bluejay");
    }

    public void testGetMwi() {
        assertNull(m_defaults.getMwiSubscribe());
        m_line.setUser(m_user);
        assertEquals("bluejay", m_defaults.getMwiSubscribe());
    }
}
