/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.rest;

import java.util.ArrayList;
import java.util.Collection;

import org.sipfoundry.sipxconfig.phonebook.Phonebook;
import org.sipfoundry.sipxconfig.phonebook.PhonebookManager;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;

public class UserPhonebookResourceTest extends UserPhonebookSearchResourceTest {

    @Override
    protected void setUp() throws Exception {
        Phonebook phonebook = new Phonebook();
        Collection<Phonebook> phonebooks = new ArrayList<Phonebook>();
        phonebooks.add(phonebook);

        m_phonebookManager = createMock(PhonebookManager.class);

        m_phonebookManager.getPhonebooksByUser(null);
        expectLastCall().andReturn(phonebooks);
        m_phonebookManager.getEntries(phonebooks);
        expectLastCall().andReturn(getMockPhonebookEntries());

        replay(m_phonebookManager);

        m_resource = new UserPhonebookResource();
        UserPhonebookResource resource = (UserPhonebookResource)m_resource;
        resource.setPhonebookManager(m_phonebookManager);
    }
}
