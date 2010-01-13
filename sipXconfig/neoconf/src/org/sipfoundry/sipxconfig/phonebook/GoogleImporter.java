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

import java.io.IOException;
import java.net.URL;
import java.util.ArrayList;
import java.util.List;

import com.google.gdata.client.Query;
import com.google.gdata.client.contacts.ContactsService;
import com.google.gdata.data.contacts.ContactEntry;
import com.google.gdata.data.contacts.ContactFeed;
import com.google.gdata.util.AuthenticationException;
import com.google.gdata.util.ServiceException;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.common.UserException;

/**
 * Extracts contact information from google address book
 */
public class GoogleImporter {
    private static final Log LOG = LogFactory.getLog(GoogleImporter.class);

    private static final String GMAIL_URL = "http://www.google.com/m8/feeds/contacts/%s/full";

    private final String m_password;

    private final String m_account;

    public GoogleImporter(String account, String password) {
        m_account = account;
        m_password = password;
    }

    List<ContactEntry> getEntries() {
        List<ContactEntry> entries = new ArrayList<ContactEntry>();
        try {
            ContactsService gmailService = new ContactsService("sipxconfig-phonebooks-4.2");
            gmailService.setUserCredentials(m_account, m_password);

            URL feedUrl = new URL(String.format(GMAIL_URL, m_account));
            Query query = new Query(feedUrl);
            query.setMaxResults(2000);
            ContactFeed resultFeed = gmailService.query(query, ContactFeed.class);
            entries = resultFeed.getEntries();
        } catch (AuthenticationException e) {
            LOG.warn("Authentication problems", e);
            throw new GmailAuthUserException();
        } catch (ServiceException e) {
            LOG.warn("Google service problems", e);
            throw new GmailServiceUserException(e);
        } catch (IOException e) {
            LOG.warn("Connectivity problems", e);
            throw new GmailTransportUserException(e);
        }

        return entries;
    }

    public int addEntries(Phonebook phonebook) {
        List<ContactEntry> entries = getEntries();
        for (ContactEntry entry : entries) {
            PhonebookGmailEntryHelper entryHelper = new PhonebookGmailEntryHelper(entry);
            PhonebookEntry phonebookEntry = entryHelper.getPhonebookEntry();
            phonebookEntry.setPhonebook(phonebook);
            phonebook.addEntry(phonebookEntry);
        }
        return entries.size();
    }

    public static class GmailAuthUserException extends UserException {

        public GmailAuthUserException() {
            super("&msg.phonebookGmailAuthError");
        }

    }

    public static class GmailServiceUserException extends UserException {

        public GmailServiceUserException(ServiceException e) {
            super("&msg.phonebookGmailError", e);
        }

    }

    public static class GmailTransportUserException extends UserException {

        public GmailTransportUserException(IOException e) {
            super("&msg.phonebookGmailTransportError", e);
        }

    }
}
