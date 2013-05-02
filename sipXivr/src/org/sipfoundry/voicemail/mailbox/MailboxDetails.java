/**
 *
 *
 * Copyright (c) 2011 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
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
    private Integer m_heardCount;
    private Integer m_unheardCount;

    public MailboxDetails(String username, List<String> inbox, List<String> saved, List<String> deleted, List<String> conference,
            List<String> unheard) {
        m_userName = username;
        m_inbox = inbox;
        m_saved = saved;
        m_deleted = deleted;
        m_conferences = conference;
        m_unheard = unheard;
        m_heardCount = m_inbox.size() - m_unheard.size();
        m_unheardCount = m_unheard.size();
    }

    public MailboxDetails(String username, int heard, int unheard) {
        m_userName = username;
        m_heardCount = heard;
        m_unheardCount = unheard;
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
        return m_unheardCount;
    }

    public int getHeardCount() {
        return m_heardCount;
    }

    public int getDeletedCount() {
        return m_deleted.size();
    }

    public int getConferencesCount() {
        return m_conferences.size();
    }

    public void incrementHeardCount() {
        m_heardCount ++;
    }

    public void incrementUnheardCount() {
        m_unheardCount ++;
    }

}
