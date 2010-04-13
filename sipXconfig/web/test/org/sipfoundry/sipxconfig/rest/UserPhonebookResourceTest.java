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
import org.sipfoundry.sipxconfig.test.TestUtil;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;

public class UserPhonebookResourceTest extends UserPhonebookSearchResourceTest {

    @Override
    protected void setUp() throws Exception {
        Collection<Phonebook> phonebooks = TestUtil.getMockAllPhonebooks();

        m_phonebookManager = createMock(PhonebookManager.class);

        m_phonebookManager.getAllPhonebooksByUser(null);
        expectLastCall().andReturn(phonebooks);
        m_phonebookManager.getEntries(phonebooks, m_user);
        expectLastCall().andReturn(getMockPhonebookEntries());

        replay(m_phonebookManager);

        m_resource = new UserPhonebookResource();
        UserPhonebookResource resource = (UserPhonebookResource)m_resource;
        resource.setPhonebookManager(m_phonebookManager);
    }
}
