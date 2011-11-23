/*
 *
 *
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.voicemail.mailbox;

import java.util.List;

public class MailboxDetails {

    private String m_userName;
    private List<String> m_inbox;
    private List<String> m_saved;
    private List<String> m_deleted;
    private List<String> m_conferences;
    private List<String> m_unheard;

    public MailboxDetails(String username, List<String> inbox, List<String> saved, List<String> deleted, List<String> conference,
            List<String> unheard) {
        m_userName = username;
        m_inbox = inbox;
        m_saved = saved;
        m_deleted = deleted;
        m_conferences = conference;
        m_unheard = unheard;
    }

    public List<String> getInbox() {
        return m_inbox;
    }

    public List<String> getSaved() {
        return m_saved;
    }

    public List<String> getDeleted() {
        return m_deleted;
    }

    public List<String> getConferences() {
        return m_conferences;
    }

    public List<String> getUnheard() {
        return m_unheard;
    }

    public String getUserName() {
        return m_userName;
    }

    public int getInboxCount() {
        return m_inbox.size();
    }

    public int getSavedCount() {
        return m_saved.size();
    }

    public int getUnheardCount() {
        return m_unheard.size();
    }

    public int getHeardCount() {
        return m_inbox.size() - m_unheard.size();
    }

    public int getDeletedCount() {
        return m_deleted.size();
    }

    public int getConferencesCount() {
        return m_conferences.size();
    }

}
