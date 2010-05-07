/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.phonebook;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.common.User;

public class GravatarTest extends TestCase {

    public void testGetUrl() {
        User user = new User();
        user.setEmailAddress("iHaveAn@email.com");

        Gravatar gravatar = new Gravatar(user);
        String url = gravatar.getUrl();
        assertEquals("https://secure.gravatar.com/avatar/3b3be63a4c2a439b013787725dfce802?s=80&d=G", url);

        gravatar = new Gravatar("iHaveAn@email.com");
        url = gravatar.getUrl();
        assertEquals("https://secure.gravatar.com/avatar/3b3be63a4c2a439b013787725dfce802?s=80&d=G", url);

        gravatar.setSize(100);
        gravatar.setType(Gravatar.DefaultType.monsterid);
        url = gravatar.getUrl();
        assertEquals("https://secure.gravatar.com/avatar/3b3be63a4c2a439b013787725dfce802?s=100&d=monsterid", url);

        String signupURl = gravatar.getSignupUrl();
        assertEquals("http://en.gravatar.com/site/signup/ihavean@email.com", signupURl);
    }

    public void testGetUrlNoEmail() {
        User user = new User();

        Gravatar gravatar = new Gravatar(user);
        String url = gravatar.getUrl();
        assertNull(url);
    }
}
