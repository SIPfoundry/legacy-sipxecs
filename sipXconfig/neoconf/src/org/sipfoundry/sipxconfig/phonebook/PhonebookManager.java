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

import java.util.Collection;

import org.sipfoundry.sipxconfig.common.User;


public interface PhonebookManager  {
    public static final String CONTEXT_BEAN_NAME = "phonebookManager";
    
    public Collection<Phonebook> getPhonebooks();

    public void deletePhonebooks(Collection<Integer> ids);

    public Collection<Phonebook> getPhonebooksByUser(User user);
    
    public Phonebook getPhonebook(Integer phonebookId);
    
    public Phonebook getPhonebookByName(String name);
    
    public void savePhonebook(Phonebook phonebook);

    public String getExternalUsersDirectory();
    
    public Collection<PhonebookEntry> getRows(Collection<Phonebook> phonebook);

    public Collection<PhonebookEntry> getRows(Phonebook phonebook);
    
    public void reset();
}
