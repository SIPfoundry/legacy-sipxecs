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

import java.io.IOException;
import java.io.OutputStream;
import java.util.Collection;

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
    public boolean getPhonebookManagementEnabled();

    public Collection<Phonebook> getPhonebooks();

    public void deletePhonebooks(Collection<Integer> ids);

    public Collection<Phonebook> getPhonebooksByUser(User user);

    public Phonebook getPhonebook(Integer phonebookId);

    public Phonebook getPhonebookByName(String name);

    public void savePhonebook(Phonebook phonebook);

    public String getExternalUsersDirectory();

    public Collection<PhonebookEntry> getEntries(Collection<Phonebook> phonebook);

    public Collection<PhonebookEntry> getEntries(Phonebook phonebook);

    public Collection<PhonebookEntry> search(Collection<Phonebook> phonebooks, String searchTerm);

    public void reset();

    public void checkPhonebookCsvFileFormat(String filename);

    public void exportPhonebook(Collection<PhonebookEntry> entries, OutputStream out) throws IOException;
}
