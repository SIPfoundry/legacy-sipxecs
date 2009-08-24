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

import junit.framework.TestCase;
import org.acegisecurity.GrantedAuthority;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.security.UserDetailsImpl;
import org.sipfoundry.sipxconfig.security.UserRole;

public class UserSessionTest extends TestCase {

    public void testLogin() {
        final User user = new User();
        user.setUniqueId();
        user.setUserName("bongo");

        UserSession userSession = new UserSession() {
            @Override
            protected UserDetailsImpl getUserDetails() {
                GrantedAuthority[] gas = {
                    UserRole.User.toAuth(), UserRole.Admin.toAuth()
                };
                return new UserDetailsImpl(user, "kuku", gas);
            }
        };

        assertTrue(userSession.isAdmin());
        assertFalse(userSession.isSupervisor());
        assertFalse(userSession.isAgent());
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
        assertFalse(userSession.isSupervisor());
        assertFalse(userSession.isAgent());
        assertFalse(userSession.isLoggedIn());
    }

    public void testNavigationVisible() throws Exception {
        // by default new Visit object has navigation enabled
        assertTrue(new UserSession().isNavigationVisible());
    }
}
