/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.phonebook;

/**
 * Represents a phonebook entry imported from a google acount
 */
public class GooglePhonebookEntry extends PhonebookEntry {
    private String m_googleAccount;

    public String getGoogleAccount() {
        return m_googleAccount;
    }

    public void setGoogleAccount(String googleAccount) {
        m_googleAccount = googleAccount;
    }
}
