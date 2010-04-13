/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */

package org.sipfoundry.sipxconfig.im;

import junit.framework.TestCase;
import org.sipfoundry.sipxconfig.common.User;

public class ImAccountTest extends TestCase {

    public void testGetImId() throws Exception {
        User user = new User();
        ImAccount imAccount = new ImAccount(user);

        user.setUserName("555");
        assertEquals("555", imAccount.getImId());

        user.addAlias("444");
        assertEquals("555", imAccount.getImId());

        user.addAlias("bongo");
        assertEquals("bongo", imAccount.getImId());

        user.addAlias("7777");
        assertEquals("bongo", imAccount.getImId());

        user.setUserName("kuku");
        assertEquals("kuku", imAccount.getImId());

        user.setImId("im");
        assertEquals("im", imAccount.getImId());
    }

    public void testSetImId() throws Exception {
        User user = new User();
        ImAccount imAccount = new ImAccount(user);

        user.setUserName("555");
        assertEquals("555", imAccount.getImId());

        imAccount.setImId("777");
        assertEquals("777", imAccount.getImId());
        assertEquals("777", user.getImId());

        imAccount.setImId("555");
        assertEquals("555", imAccount.getImId());
        assertNotNull(user.getImId());
    }

    public void testGetImDisplayName() throws Exception {
        User user = new User();
        ImAccount imAccount = new ImAccount(user);

        user.setUserName("joe");
        assertEquals("joe", imAccount.getImDisplayName());

        user.setFirstName("first");
        user.setLastName("last");
        assertEquals("first last", imAccount.getImDisplayName());

        user.setImDisplayName("joe im");
        assertEquals("joe im", imAccount.getImDisplayName());
    }

    public void testSetImDisplayName() throws Exception {
        User user = new User();
        ImAccount imAccount = new ImAccount(user);

        user.setUserName("jane");
        assertEquals("jane", imAccount.getImDisplayName());
        assertNull(user.getImDisplayName());

        imAccount.setImDisplayName("Jane User");
        assertEquals("Jane User", imAccount.getImDisplayName());
        assertEquals("Jane User", user.getImDisplayName());

        imAccount.setImDisplayName("jane");
        assertEquals("jane", imAccount.getImDisplayName());
        assertNull(user.getImDisplayName());
    }

    public void testGeImPassword() {
        User user = new User();
        ImAccount imAccount = new ImAccount(user);

        user.setUserName("555");
        assertEquals("555", imAccount.getImPassword());

        user.setImPassword("pwd");
        assertEquals("pwd", imAccount.getImPassword());

        user.setImPassword("  ");
        assertEquals("555", imAccount.getImPassword());
    }
}
