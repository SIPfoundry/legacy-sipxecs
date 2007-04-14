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
        };

        SpeedDial sd = new SpeedDial();
        sd.setUser(user);
        assertEquals("~~rl~115", sd.getResourceListId(false));
        assertEquals("~~rl~115c", sd.getResourceListId(true));
    }
}
