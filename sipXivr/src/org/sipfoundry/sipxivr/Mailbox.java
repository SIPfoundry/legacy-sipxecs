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
import org.sipfoundry.commons.userdb.User;

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
    
    private PersonalAttendant m_personalAttendant;

    
    public Mailbox(User user) {
        init(user, IvrConfiguration.get().getMailstoreDirectory());
    }

    /**
     * Mini constructor, passed in mailstore for testing (no need for config)
     */
    public Mailbox(User user, String mailStoreDirectory) {
        init(user, mailStoreDirectory);
    }

    private void init(User user, String mailStoreDirectory) {
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
        m_lastModified = m_mailboxPreferencesFile.lastModified();
    }
    
    /**
     * Create the mailbox directory and all sub directories if needed
     */
    public synchronized static void createDirsIfNeeded(Mailbox mailbox) {
        
        File userDir = new File(mailbox.getUserDirectory());
        File inboxDir = new File(mailbox.getInboxDirectory());
        File savedDir = new File(mailbox.getSavedDirectory());
        File deletedDir = new File(mailbox.getDeletedDirectory());
        if (!userDir.isDirectory()) {
            LOG.info("Mailbox::createDirsIfNeeded creating mailbox "+userDir.getPath());
            userDir.mkdir();
        }
        if (!inboxDir.isDirectory()) {
            inboxDir.mkdir();
        }
        if (!savedDir.isDirectory()) {
            savedDir.mkdir();
        }
        if (!deletedDir.isDirectory()) {
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

    /**
     * 
     * @return the directory that contains all the mailboxes
     */
    public String getMailstoreDirectory() {
        return m_mailStoreDirectory;
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
     * Get the File with the user's recorded name.
     * 
     * @return the File
     */
    public File getRecordedNameFile() {
        // The recorded name is stored in the "name.wav" file in the user's directory
        String name = m_userDirectory + "name.wav";
        File f = new File(name);
        return f;
    }
    
    /**
     * Reads and caches the mailboxprefs.xml file
     * @return the MailboxPreferences object described in mailboxprefs.xml
     */
    public MailboxPreferences getMailboxPreferences() {
        if (m_mailboxPreferences == null || m_lastModified != m_mailboxPreferencesFile.lastModified()) {
            MailboxPreferencesReader mpr = new MailboxPreferencesReader();
            m_lastModified = m_mailboxPreferencesFile.lastModified();
            m_mailboxPreferences = mpr.readObject(m_mailboxPreferencesFile) ;
            if (m_mailboxPreferences == null) {
                m_mailboxPreferences = new MailboxPreferences();
            }
        }
        return m_mailboxPreferences ; 
    }
    
    public void writeMailboxPreferences(String pin) {
        // only preferences updated here are the active greeing and that is done
        // via a sipXconfig REST call
        
        // /sipxconfig/rest/my/mailbox/200/preferences/activegreeting/standard

        RestfulRequest rr = new RestfulRequest(
                    IvrConfiguration.get().getConfigUrl()+"/sipxconfig/rest/my/mailbox/" + 
                    m_user.getUserName() + "/preferences/activegreeting/", 
                    m_user.getUserName(), pin);
                   
        try {
            if (rr.put(getMailboxPreferences().getActiveGreeting().getActiveGreeting())) {
                LOG.info("Mailbox::writeMailboxPreferences:change Greeting "+m_user.getUserName()+" greeting changed.");
            }
        } catch (Exception e) {
            LOG.info("Mailbox::writeMailboxPreferences:change Greeting "+m_user.getUserName()+" failed: " + e.getMessage());
        }        
    }
    
    public long getLastModified() {
        return m_lastModified;
    }

    /**
     * The file that holds the distribution lists
     * @return the file.
     */

    public File getDistributionListsFile() {
        return new File(getUserDirectory(), "distribution.xml");
    }

    /**
     * Load the personal attendant for this mailbox if not already done so.
     * @return
     */
    public PersonalAttendant getPersonalAttendant() {
        if (m_personalAttendant == null) {
            m_personalAttendant = new PersonalAttendant(this);
        }
        return m_personalAttendant;
    }
    
}
