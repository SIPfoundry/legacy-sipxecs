/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.site;

import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.classextension.EasyMock.createMock;

import java.util.ArrayList;
import java.util.Collection;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.permission.PermissionManager;
import org.sipfoundry.sipxconfig.security.UserDetailsImpl;
import org.sipfoundry.sipxconfig.security.UserRole;
import org.sipfoundry.sipxconfig.test.TestHelper;
import org.springframework.security.core.GrantedAuthority;

public class UserSessionTest extends TestCase {

    public void testLogin() {
        final User user = new User();
        user.setUniqueId();
        user.setUserName("bongo");
        PermissionManager pManager = createMock(PermissionManager.class);
        pManager.getPermissionModel();
        expectLastCall().andReturn(TestHelper.loadSettings("commserver/user-settings.xml")).anyTimes();
        replay(pManager);
        user.setPermissionManager(pManager);
        UserSession userSession = new UserSession() {
            @Override
            protected UserDetailsImpl getUserDetails() {
                Collection<GrantedAuthority> gas = new ArrayList<GrantedAuthority>(2);
                gas.add(UserRole.User.toAuth());
                gas.add(UserRole.Admin.toAuth());
                return new UserDetailsImpl(user, "kuku", gas, true);
            }
        };

        assertTrue(userSession.isAdmin());
        assertTrue(userSession.isLoggedIn());

        assertSame(user.getId(), userSession.getUserId());
    }

    public void testNoLogin() {
        UserSession userSession = new UserSession() {
            @Override
            protected UserDetailsImpl getUserDetails() {
                return null;
            }
        };

        assertFalse(userSession.isAdmin());
        assertFalse(userSession.isLoggedIn());
    }

    public void testNavigationVisible() throws Exception {
        // by default new Visit object has navigation enabled
        assertTrue(new UserSession().isNavigationVisible());
    }
}
