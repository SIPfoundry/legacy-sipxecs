/*
 *
 *
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.vm;

import java.io.File;
import java.util.Arrays;
import java.util.List;

public class LocalMailbox {
    private File m_mailstoreDirectory;
    private String m_userId;

    public LocalMailbox(File mailstoreDirectory, String userId) {
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
}
