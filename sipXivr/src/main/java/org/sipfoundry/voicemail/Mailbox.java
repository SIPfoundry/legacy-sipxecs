/*
 * 
 * 
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 */
package org.sipfoundry.voicemail;

import java.io.File;
import java.io.IOException;

import org.apache.log4j.Logger;
import org.sipfoundry.sipxivr.Localization;
import org.sipfoundry.sipxivr.User;

public class Mailbox {
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxivr");
    private User m_user;
    
    private MailboxPreferences m_mailboxPreferences;
    private File m_mailboxPreferencesFile;
    private long m_lastModified;
    
    private String m_mailStoreDirectory;
    private String m_userDirectory;
    private Localization m_localization;

    Mailbox(User user, Localization localization) {
        m_user = user ;
        m_localization = localization;
        m_mailStoreDirectory = org.sipfoundry.sipxivr.Configuration.update(true).getMailstoreDirectory();
        m_userDirectory = m_mailStoreDirectory + "/" + m_user.getUserName()+ "/";
        m_mailboxPreferencesFile = new File(m_userDirectory + "mailboxprefs.xml") ;
        
    }

    public User getUser() {
        return m_user;
    }
    
    public Localization getLocalization() {
        return m_localization;
    }
    
    public String getUserDirectory() {
        return m_userDirectory;
    }
    
    public MailboxPreferences getMailboxPreferences() {
        if (m_mailboxPreferences == null || m_lastModified != m_mailboxPreferencesFile.lastModified()) {
            MailboxPreferencesReader mpr = new MailboxPreferencesReader();
            m_lastModified = m_mailboxPreferencesFile.lastModified();
            m_mailboxPreferences = mpr.readObject(m_mailboxPreferencesFile) ;
        }
        return m_mailboxPreferences ; 
    }
    
    public void writeMailboxPreferences() {
        MailboxPreferencesWriter mpw = new MailboxPreferencesWriter() ;
        try {
            // Write a temporary file so any readers will not read incomplete data
            File tempFile = File.createTempFile("temp_mailboxprefs", ".xml", m_mailboxPreferencesFile.getParentFile());
            mpw.writeObject(m_mailboxPreferences, tempFile);
            // Move the temporary file to the correct filename
            tempFile.renameTo(m_mailboxPreferencesFile) ;
            m_lastModified = m_mailboxPreferencesFile.lastModified();
        } catch (IOException e) {
            LOG.error("writeMailboxPreferences cannot create tmp file",e);
            throw new RuntimeException(e);
        }
    }
}
