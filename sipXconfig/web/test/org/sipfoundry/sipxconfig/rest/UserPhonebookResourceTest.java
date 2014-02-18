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

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;
import static org.restlet.data.MediaType.TEXT_PLAIN;
import static org.restlet.data.MediaType.TEXT_VCARD;

import java.io.IOException;
import java.io.OutputStream;
import java.util.Collection;

import org.easymock.EasyMock;
import org.restlet.resource.ResourceException;
import org.restlet.resource.Variant;
import org.sipfoundry.sipxconfig.phonebook.Phonebook;
import org.sipfoundry.sipxconfig.phonebook.PhonebookEntry;
import org.sipfoundry.sipxconfig.phonebook.PhonebookManager;
import org.sipfoundry.sipxconfig.phonebook.PhonebookManager.PhonebookFormat;
import org.sipfoundry.sipxconfig.test.TestHelper;

public class UserPhonebookResourceTest extends UserPhonebookSearchResourceTest {

    @Override
    protected void setUp() throws Exception {
        Collection<Phonebook> phonebooks = TestHelper.getMockAllPhonebooks();

        m_phonebookManager = createMock(PhonebookManager.class);

        m_phonebookManager.getAllPhonebooksByUser(null);
        expectLastCall().andReturn(phonebooks);
        m_phonebookManager.getEntries(phonebooks, m_user);
        Collection<PhonebookEntry> entries = getMockPhonebookEntries();
        expectLastCall().andReturn(entries);

        replay(m_phonebookManager);

        m_resource = new UserPhonebookResource();
        UserPhonebookResource resource = (UserPhonebookResource) m_resource;
        resource.setPhonebookManager(m_phonebookManager);
    }

    public void testGetCsv() throws ResourceException, IOException {
        Collection<Phonebook> phonebooks = TestHelper.getMockAllPhonebooks();

        PhonebookManager mgr = exportSetup(phonebooks, PhonebookFormat.CSV);

        m_resource = new UserPhonebookResource();
        UserPhonebookResource resource = (UserPhonebookResource) m_resource;
        resource.setPhonebookManager(mgr);

        m_resource.represent(new Variant(TEXT_PLAIN));
        verify(mgr);
    }

    public void testGetVcard() throws ResourceException, IOException {
        Collection<Phonebook> phonebooks = TestHelper.getMockAllPhonebooks();

        PhonebookManager mgr = exportSetup(phonebooks, PhonebookFormat.VCARD);

        m_resource = new UserPhonebookResource();
        UserPhonebookResource resource = (UserPhonebookResource) m_resource;
        resource.setPhonebookManager(mgr);

        m_resource.represent(new Variant(TEXT_VCARD));
        verify(mgr);
    }

    private PhonebookManager exportSetup(Collection<Phonebook> phonebooks, PhonebookFormat fmt) throws IOException {
        PhonebookManager mgr = createMock(PhonebookManager.class);

        mgr.getAllPhonebooksByUser(null);
        expectLastCall().andReturn(phonebooks);
        mgr.getEntries(phonebooks, m_user);
        Collection<PhonebookEntry> entries = getMockPhonebookEntries();
        expectLastCall().andReturn(entries);
        mgr.exportPhonebook(EasyMock.eq(entries), (OutputStream) EasyMock.anyObject(), EasyMock.eq(fmt));
        expectLastCall();

        replay(mgr);
        return mgr;
    }
}
