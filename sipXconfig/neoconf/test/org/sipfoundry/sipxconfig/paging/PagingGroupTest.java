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

import org.sipfoundry.sipxconfig.common.User;

import junit.framework.TestCase;

public class PagingGroupTest extends TestCase {
    public void testFormatUrls() {
        PagingGroup group1 = new PagingGroup();
        User user1 = new User();
        user1.setUserName("userTest");
        group1.getUsers().add(user1);
        assertEquals("userTest@test.org", group1.formatUrls("test.org"));
    }

    public void testFormatBeep() {
        PagingGroup group2 = new PagingGroup();
        group2.setSound("beep.wav");
        assertEquals("file:///audio/beep.wav", group2.formatBeep("/audio"));
    }

    public void testFormatDescription() {
        PagingGroup group3 = new PagingGroup();
        group3.setDescription("description");
        assertEquals("description", group3.formatDescription());
    }

    public void testFormatPageGroupNumber() {
        PagingGroup group4 = new PagingGroup();
        group4.setPageGroupNumber(Long.valueOf(43));
        assertEquals("43", group4.formatPageGroupNumber());
    }
}
