/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.speeddial;

import org.sipfoundry.sipxconfig.common.User;

import junit.framework.TestCase;

public class SpeedDialTest extends TestCase {

    public void testGetResourceListId() {
        User user = new User() {
            public Integer getId() {
                return 115;
            }
            public String getUserName() {
                return "test_user";
            }
        };

        SpeedDial sd = new SpeedDial();
        sd.setUser(user);
        assertEquals("~~rl~F~test_user", sd.getResourceListId(false));
        assertEquals("~~rl~C~test_user", sd.getResourceListId(true));
    }
}
