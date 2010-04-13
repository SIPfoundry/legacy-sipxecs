/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.paging;

import java.util.LinkedHashSet;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.common.User;

public class PagingGroupTest extends TestCase {
    public void testFormatUrls() {
        PagingGroup group = new PagingGroup();
        group.setUsers(new LinkedHashSet<User>());

        assertEquals("", group.formatUserList("test.org"));

        User user1 = new User();
        user1.setUniqueId();
        user1.setUserName("abc");
        group.getUsers().add(user1);
        assertEquals("abc@test.org", group.formatUserList("test.org"));

        User user2 = new User();
        user2.setUniqueId();
        user2.setUserName("cde");
        group.getUsers().add(user2);

        // order does not have to match
        assertEquals("abc@example.org,cde@example.org", group.formatUserList("example.org"));
    }
}
