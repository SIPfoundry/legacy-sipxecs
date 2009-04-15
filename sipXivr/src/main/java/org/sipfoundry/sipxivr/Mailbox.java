/*
 * 
 * 
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 */
package org.sipfoundry.sipxivr;

import java.io.File;
import java.io.IOException;

import org.apache.log4j.Logger;

public class Mailbox {
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxivr");
    private User m_user;
    
    private MailboxPreferences m_mailboxPreferences;
    private File m_mailboxPreferencesFile;
    private long m_lastModified;
    
    private String m_mailStoreDirectory;
    private String m_userDirectory;
    private String m_inboxDirectory;
    private String m_savedDirectory;
    private String m_deletedDirectory;
    private Localization m_localization;

    public Mailbox(User user, Localization localization) {
        m_user = user ;
        m_localization = localization;
        m_mailStoreDirectory = org.sipfoundry.sipxivr.Configuration.update(true).getMailstoreDirectory();
        defineDirs();
    }

    /**
     * Mini constructor, used for testing (passed in mailstore, no localization)
     */
    public Mailbox(User user, String mailStoreDirectory) {
        m_user = user;
        m_mailStoreDirectory = mailStoreDirectory;
        defineDirs();
    }

    void defineDirs() {
        m_userDirectory = m_mailStoreDirectory + "/" + m_user.getUserName()+ "/";
        m_inboxDirectory = m_userDirectory+"inbox/";
        m_savedDirectory = m_userDirectory+"saved/";
        m_deletedDirectory = m_userDirectory+"deleted/";

        m_mailboxPreferencesFile = new File(m_userDirectory + "mailboxprefs.xml") ;
    }
    
    /**
     * Create the mailbox directory and all sub directories if needed
     */
    public synchronized static void createDirsIfNeeded(Mailbox mailbox) {
        
        File userDir = new File(mailbox.getUserDirectory());
        if (!userDir.isDirectory()) {
            userDir.mkdir();
            File inboxDir = new File(mailbox.getInboxDirectory());
            File savedDir = new File(mailbox.getSavedDirectory());
            File deletedDir = new File(mailbox.getDeletedDirectory());
            inboxDir.mkdir();
            savedDir.mkdir();
            deletedDir.mkdir();
        }
    }
    /**
     * 
     * @return the User of this mailbox
     */
    public User getUser() {
        return m_user;
    }
    
    public Localization getLocalization() {
        return m_localization;
    }
    
    /**
     * 
     * @return the directory that contains this mailbox
     */
    public String getUserDirectory() {
        return m_userDirectory;
    }

    /**
     * 
     * @return the directory that contains the inbox
     */
    public String getInboxDirectory() {
        return m_inboxDirectory;
    }

    /**
     * 
     * @return the directory that contains the saved box
     */
    public String getSavedDirectory() {
        return m_savedDirectory;
    }

    /**
     * 
     * @return the directory that contains this deleted box
     */
    public String getDeletedDirectory() {
        return m_deletedDirectory;
    }

    /**
     * Get the path with the user's recorded name.
     * 
     * @return the path, or null if it doesn't exist.
     */
    public String getRecordedName() {
        // The recorded name is stored in the "name.wav" file in the user's directory
        String name = m_userDirectory + "name.wav";
        File f = new File(name);
        if (f.exists()) {
            return name;
        }
        return null;
    }
    
    /**
     * Reads and caches the mailboxprefs.xml file
     * @return the MailboxPreferences object described in mailboxprefs.xml
     */
    public MailboxPreferences getMailboxPreferences() {
        createDirsIfNeeded(this);
        if (m_mailboxPreferences == null || m_lastModified != m_mailboxPreferencesFile.lastModified()) {
            MailboxPreferencesReader mpr = new MailboxPreferencesReader();
            m_lastModified = m_mailboxPreferencesFile.lastModified();
            m_mailboxPreferences = mpr.readObject(m_mailboxPreferencesFile) ;
        }
        return m_mailboxPreferences ; 
    }
    
    public void writeMailboxPreferences() {
        createDirsIfNeeded(this);
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

    /**
     * The file that holds the distribution lists
     * @return the file.
     */

    public File getDistributionListsFile() {
        return new File(getUserDirectory(), "distribution.xml");
    }
    
}
