/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.phonebook;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.Collection;
import java.util.Map;

import org.sipfoundry.sipxconfig.common.DataObjectSource;
import org.sipfoundry.sipxconfig.common.User;

public interface PhonebookManager extends DataObjectSource<Phonebook> {
    public static final String CONTEXT_BEAN_NAME = "phonebookManager";

    /**
     * Gets whether or not phonebook management is enabled.
     *
     * If phonebook management is disabled, no phonebook files will be generated for any phones
     * that use a separate phonebook file.
     */
    boolean getPhonebookManagementEnabled();

    Collection<Phonebook> getPhonebooks();

    void deletePhonebooks(Collection<Integer> ids);

    Collection<Phonebook> getPhonebooksByUser(User user);

    Phonebook getPhonebook(Integer phonebookId);

    Phonebook getPhonebookByName(String name);

    void savePhonebook(Phonebook phonebook);

    String getExternalUsersDirectory();

    Collection<PhonebookEntry> getEntries(Collection<Phonebook> phonebook, User user);

    PagedPhonebook getPagedPhonebook(Collection<Phonebook> phonebook, User user, String startRow, String endRow,
            String queryString);

    Collection<PhonebookEntry> getEntries(Phonebook phonebook);

    Collection<PhonebookEntry> search(Collection<Phonebook> phonebooks, String searchTerm, User user);

    void reset();

    void exportPhonebook(Collection<PhonebookEntry> entries, OutputStream out) throws IOException;

    int addEntriesFromFile(Integer phonebookId, InputStream in);

    int addEntriesFromGmailAccount(Integer phonebookId, String account, String password);

    Phonebook getPrivatePhonebook(User user);

    PhonebookEntry getPhonebookEntry(Integer phonebookFileEntryId);

    void savePhonebookEntry(PhonebookEntry entry);

    void deletePhonebookEntry(PhonebookEntry entry);

    // --> this methods will be removed after version update
    void removeTableColumns();

    Map<Integer, String[]> getPhonebookFilesName();
    // <--
}
