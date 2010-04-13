/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.vm;

import java.io.File;
import java.io.IOException;
import java.util.Arrays;
import java.util.List;

import org.apache.commons.io.FileUtils;
import org.sipfoundry.sipxconfig.vm.MailboxManagerImpl.MailstoreMisconfigured;

public class Mailbox {
    private File m_mailstoreDirectory;
    private String m_userId;

    public Mailbox(File mailstoreDirectory, String userId) {
        m_mailstoreDirectory = mailstoreDirectory;
        m_userId = userId;
    }

    public List<String> getFolderIds() {
        // to support custom folders, return these names and any additional
        // directories here
        return Arrays.asList(new String[] {"inbox", "conference", "deleted", "saved"});
    }

    public File getMailstoreDirectory() {
        return m_mailstoreDirectory;
    }

    public File getUserDirectory() {
        return new File(getMailstoreDirectory(), getUserId());
    }

    public void deleteUserDirectory() {
        File userDir = getUserDirectory();
        if (userDir.exists()) {
            try {
                FileUtils.deleteDirectory(userDir);
            } catch (IOException e) {
                throw new MailstoreMisconfigured("Cannot delete mailbox directory " + userDir.getAbsolutePath(), e);
            }
        }
    }

    public File getDistributionListsFile() {
        return new File(getUserDirectory(), "distribution.xml");
    }

    public void setMailstoreDirectory(File mailstoreDirectory) {
        m_mailstoreDirectory = mailstoreDirectory;
    }

    public String getUserId() {
        return m_userId;
    }

    public void setUserId(String userId) {
        m_userId = userId;
    }

    public File getVoicemailPreferencesFile() {
        return new File(getUserDirectory(), "mailboxprefs.xml");
    }

    public Voicemail getVoicemail(String folderId, String messageId) {
        return new Voicemail(m_mailstoreDirectory, m_userId, folderId, messageId);
    }
}
