/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.bulk;

import java.util.Collection;

import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.vm.MailboxPreferences;

/**
 * Used to preview users that are imported but not added to the database
 */
public class UserPreview {
    private User m_user;
    private MailboxPreferences m_mailboxPreferences;
    private Collection<String> m_groupNames;

    public UserPreview(User user, Collection<String> groupNames, MailboxPreferences preferences) {
        m_user = user;
        m_groupNames = groupNames;
        // non-null preferences easier to preview and accurate as defaults
        m_mailboxPreferences = preferences == null ? new MailboxPreferences() : preferences;
    }

    public User getUser() {
        return m_user;
    }
    
    public MailboxPreferences getMailboxPreferences() {
        return m_mailboxPreferences;
    }

    public Collection<String> getGroupNames() {
        return m_groupNames;
    }
}
