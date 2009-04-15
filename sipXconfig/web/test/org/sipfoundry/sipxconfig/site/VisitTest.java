/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.common.User;

public class VisitTest extends TestCase {

    public void testLogin() {
        User user = new User();
        UserSession userSession = new UserSession();
        assertNull(userSession.getUserId());

        userSession.login(user.getId(), false, true, true);

        assertSame(user.getId(), userSession.getUserId());
        assertFalse(userSession.isAdmin());

        userSession.login(user.getId(), true, false, true);
        assertSame(user.getId(), userSession.getUserId());
        assertTrue(userSession.isAdmin());
    }

    public void testNavigationVisible() throws Exception {
        // by default new Visit object has navigation enabled
        assertTrue(new UserSession().isNavigationVisible());
    }
}
