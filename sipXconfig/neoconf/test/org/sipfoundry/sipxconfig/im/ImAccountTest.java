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
        user.setImId(null);

        user.addAlias("444");
        assertEquals("555", imAccount.getImId());
        user.setImId(null);

        user.addAlias("bongo");
        assertEquals("bongo", imAccount.getImId());
        user.setImId(null);

        user.addAlias("7777");
        assertEquals("bongo", imAccount.getImId());
        user.setImId(null);

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
        assertEquals("555", user.getImId());

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
        assertNull(imAccount.getImDisplayName());

        user.setImId("joe im id");
        assertEquals("joe im id", imAccount.getImDisplayName());

        user.setFirstName("first");
        user.setLastName("last");
        assertEquals("first last", imAccount.getImDisplayName());

        user.setImDisplayName("joe im");
        assertEquals("joe im", imAccount.getImDisplayName());
    }

    public void testDisplayNameComponents() throws Exception {
        User user = new User();
        ImAccount imAccount = new ImAccount(user);

        user.setFirstName("John");
        assertEquals("John", imAccount.getImDisplayName());
        assertEquals("John", user.getImDisplayName());
        user.setLastName("Fitzgerald");
        assertEquals("John Fitzgerald", imAccount.getImDisplayName());
        assertEquals("John Fitzgerald", user.getImDisplayName());
        user.setFirstName(null);
        user.setLastName("Doe");
        assertEquals("Doe", imAccount.getImDisplayName());
        assertEquals("Doe", user.getImDisplayName());
    }

    public void testSetImDisplayName() throws Exception {
        User user = new User();
        ImAccount imAccount = new ImAccount(user);

        user.setUserName("jane");
        assertNull(imAccount.getImDisplayName());
        assertNull(user.getImDisplayName());

        user.setImId("jane im id");
        assertEquals("jane im id", imAccount.getImDisplayName());
        assertEquals("jane im id", user.getImDisplayName());

        imAccount.setImDisplayName("Jane User");
        assertEquals("Jane User", imAccount.getImDisplayName());
        assertEquals("Jane User", user.getImDisplayName());

        imAccount.setImDisplayName("jane");
        assertEquals("jane", imAccount.getImDisplayName());
        //we need to always set imDisplayName even if is equal with username.
        //we need it replicated in potential plugins for vcard in openfire
        assertEquals("jane", user.getImDisplayName());
    }
}
