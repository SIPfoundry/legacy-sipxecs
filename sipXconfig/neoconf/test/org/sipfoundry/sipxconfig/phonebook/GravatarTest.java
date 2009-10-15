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
import org.sipfoundry.sipxconfig.vm.MailboxManager;
import org.sipfoundry.sipxconfig.vm.MailboxPreferences;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;

public class GravatarTest extends TestCase {

    public void testGetUrl() {
        User user = new User();

        MailboxPreferences mailboxPreferences = new MailboxPreferences();
        mailboxPreferences.setEmailAddress("iHaveAn@email.com");

        MailboxManager mailboxManager = createMock(MailboxManager.class);
        mailboxManager.getMailboxPreferencesForUser(user);
        expectLastCall().andReturn(mailboxPreferences).times(3);
        replay(mailboxManager);

        Gravatar gravatar = new Gravatar(user);
        String url = gravatar.getUrl(mailboxManager);
        assertEquals("http://www.gravatar.com/avatar/3b3be63a4c2a439b013787725dfce802?s=80&d=wavatar", url);

        gravatar.setSize(100);
        gravatar.setType(Gravatar.DefaultType.monsterid);
        url = gravatar.getUrl(mailboxManager);
        assertEquals("http://www.gravatar.com/avatar/3b3be63a4c2a439b013787725dfce802?s=100&d=monsterid", url);

        String signupURl = gravatar.getSignupUrl(mailboxManager);
        assertEquals("http://en.gravatar.com/site/signup/ihavean@email.com", signupURl);

        verify(mailboxManager);
    }

    public void testGetUrlNoEmail() {
        User user = new User();

        MailboxPreferences mailboxPreferences = new MailboxPreferences();

        MailboxManager mailboxManager = createMock(MailboxManager.class);
        mailboxManager.getMailboxPreferencesForUser(user);
        expectLastCall().andReturn(mailboxPreferences);
        replay(mailboxManager);

        Gravatar gravatar = new Gravatar(user);
        String url = gravatar.getUrl(mailboxManager);
        assertNull(url);

        verify(mailboxManager);
    }

}
