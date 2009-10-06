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

import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.vm.MailboxManager;
import org.sipfoundry.sipxconfig.vm.MailboxPreferences;

import static org.apache.commons.codec.digest.DigestUtils.md5Hex;

/**
 * Calculates URL for the users avatar provided by gravatar.com service
 */
public class Gravatar {
    private final User m_user;

    public Gravatar(User user) {
        m_user = user;
    }

    /**
     * Calculates URL for the users avatar provided by gravatar.com service
     *
     * @param mailboxManager until we have independent source of mailbox preferences we need to
     *        pass mailbox manager around
     * @return URL that can be used to retrieve gravatar
     */
    public String getUrl(MailboxManager mailboxManager) {
        String email = getEmail(mailboxManager);
        if (email == null) {
            return null;
        }

        return String.format("http://www.gravatar.com/avatar/%s", md5Hex(email.toLowerCase()));
    }

    private String getEmail(MailboxManager mailboxManager) {
        MailboxPreferences mailboxPreferences = mailboxManager.getMailboxPreferencesForUser(m_user);
        return mailboxPreferences.getEmailAddress();
    }
}
