/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site.user;

import junit.framework.TestCase;

import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.permission.PermissionName;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class PinTokenChangeServletTest extends TestCase {

    private PinTokenChangeServlet m_servlet;

    private User m_user;

    protected void setUp() {
        m_servlet = new PinTokenChangeServlet();
        m_user = new User() {
            protected Setting loadSettings() {
                return SiteTestHelper.loadSettings("commserver/user-settings.xml");
            }
        };
        m_user.setUserName("joe");
        m_user.setPintoken("oldpintoken");
    }

    public void testIllformatted() {
        try {
            m_servlet.changePin(null, "userid:oldpintoken:newpintoken");
            fail();
        } catch (PinTokenChangeServlet.ChangePinException expected) {
            assertTrue(true);
        }

        try {
            m_servlet.changePin(null, "userid;oldpintoken");
            fail();
        } catch (PinTokenChangeServlet.ChangePinException expected) {
            assertTrue(true);
        }
    }

    public void testNoUser() {
        IMocksControl coreContextCtrl = EasyMock.createControl();
        CoreContext coreContext = coreContextCtrl.createMock(CoreContext.class);
        coreContext.loadUserByUserName("joe");
        coreContextCtrl.andReturn(null);
        coreContextCtrl.replay();

        try {
            m_servlet.changePin(coreContext, "joe;oldpintoken;newpintoken");
            fail();
        } catch (PinTokenChangeServlet.ChangePinException expected) {
            assertTrue(true);
        }

        coreContextCtrl.verify();
    }

    public void testChangePin() {
        IMocksControl coreContextCtrl = EasyMock.createControl();
        CoreContext coreContext = coreContextCtrl.createMock(CoreContext.class);
        coreContext.loadUserByUserName("joe");
        coreContextCtrl.andReturn(m_user);
        coreContext.saveUser(m_user);
        coreContextCtrl.andReturn(false);
        coreContextCtrl.replay();

        m_servlet.changePin(coreContext, "joe;oldpintoken;newpintoken");

        coreContextCtrl.verify();
        assertEquals("newpintoken", m_user.getPintoken());
    }

    public void testNotProviledgedToChangePin() {
        Group g = new Group();
        m_user.addGroup(g);
        PermissionName.TUI_CHANGE_PIN.setEnabled(g, false);

        IMocksControl coreContextCtrl = EasyMock.createControl();
        CoreContext coreContext = coreContextCtrl.createMock(CoreContext.class);
        coreContext.loadUserByUserName("joe");
        coreContextCtrl.andReturn(m_user);
        coreContextCtrl.replay();

        try {
            m_servlet.changePin(coreContext, "joe;oldpintoken;newpintoken");
            fail();
        } catch (PinTokenChangeServlet.ChangePinException expected) {
            assertTrue(true);
        }

        coreContextCtrl.verify();
    }
}
