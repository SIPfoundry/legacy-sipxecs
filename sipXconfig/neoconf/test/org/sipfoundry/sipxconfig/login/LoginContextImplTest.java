/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.login;

import junit.framework.TestCase;

import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.permission.PermissionManagerImpl;
import org.sipfoundry.sipxconfig.permission.PermissionName;
import org.sipfoundry.sipxconfig.setting.Group;

public class LoginContextImplTest extends TestCase {
    private LoginContextImpl m_impl;
    private User m_user;
    private static final Integer USER_ID = new Integer(314159);
    private PermissionManagerImpl m_permissionManager;

    protected void setUp() throws Exception {
        m_impl = new LoginContextImpl();

        m_permissionManager = new PermissionManagerImpl();
        m_permissionManager.setModelFilesContext(TestHelper.getModelFilesContext());

        m_user = new User();
        m_user.setPermissionManager(m_permissionManager);

        Group admin = new Group();
        m_user.addGroup(admin);

        PermissionName.SUPERADMIN.setEnabled(admin, true);

        IMocksControl control = EasyMock.createNiceControl();
        CoreContext coreContext = control.createMock(CoreContext.class);
        coreContext.loadUserByUserNameOrAlias("superadmin");
        control.andReturn(m_user).times(3);

        coreContext.getAuthorizationRealm();
        control.andReturn("pingtel.com").times(3);

        coreContext.loadUser(USER_ID);
        control.andReturn(m_user).times(1);
        control.replay();

        m_impl.setCoreContext(coreContext);
    }

    public void testCheckCredentials() {
        m_user.setUserName("superadmin");
        m_user.setPintoken("e3e367205de83ab477cdf3449f152791");

        // password OK
        assertSame(m_user, m_impl.checkCredentials("superadmin", "1234"));

        // MD5 hash passed - should fail
        assertNull(m_impl.checkCredentials("superadmin", "e3e367205de83ab477cdf3449f152791"));

        // invalid password
        assertNull(m_impl.checkCredentials("superadmin", "123"));
    }

    public void testCheckCredentialsInvalid() {
        m_user.setUserName("superadmin");
        m_user.setPintoken("kkk");

        assertNull(m_impl.checkCredentials("superadmin", "zzz"));

        // invalid user - good password
        assertNull(m_impl.checkCredentials("xxx", "kkk"));

    }

    public void testIsAdmin() {
        m_user.setUserName("superadmin");
        assertTrue(m_impl.isAdmin(m_user));

        m_user.setUserName("xyz");
        assertTrue(m_impl.isAdmin(m_user));
    }

    public void testNotAdmin() {
        User nonAdmin = new User();
        nonAdmin.setPermissionManager(m_permissionManager);
        nonAdmin.setUserName("superadmin"); // not really superadmin
        Group nonAdminGroup = new Group();
        nonAdmin.addGroup(nonAdminGroup);

        assertFalse(m_impl.isAdmin(nonAdmin));
    }

    public void testIsAdminUserId() {
        m_user.setUserName("superadmin");
        assertTrue(m_impl.isAdmin(USER_ID));
    }
}
