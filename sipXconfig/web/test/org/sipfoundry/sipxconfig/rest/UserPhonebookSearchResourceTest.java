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

import java.io.StringWriter;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;

import junit.framework.TestCase;
import org.apache.commons.io.IOUtils;
import org.restlet.data.MediaType;
import org.restlet.data.Reference;
import org.restlet.data.Request;
import org.restlet.resource.Representation;
import org.restlet.resource.Variant;
import org.sipfoundry.sipxconfig.phonebook.AddressBookEntry;
import org.sipfoundry.sipxconfig.phonebook.Phonebook;
import org.sipfoundry.sipxconfig.phonebook.PhonebookEntry;
import org.sipfoundry.sipxconfig.phonebook.PhonebookManager;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;

public class UserPhonebookSearchResourceTest extends TestCase{
    protected PhonebookManager m_phonebookManager;
    protected UserResource m_resource;
    @Override
    protected void setUp() throws Exception {

        Phonebook phonebook = new Phonebook();
        Collection<Phonebook> phonebooks = new ArrayList<Phonebook>();
        phonebooks.add(phonebook);

        m_phonebookManager = createMock(PhonebookManager.class);

        m_phonebookManager.getPhonebooksByUser(null);
        expectLastCall().andReturn(phonebooks);
        m_phonebookManager.search(phonebooks, "searchTerm");
        expectLastCall().andReturn(getMockPhonebookEntries());

        replay(m_phonebookManager);

        m_resource = new UserPhonebookSearchResource();
        UserPhonebookSearchResource resource = (UserPhonebookSearchResource) m_resource;
        resource.setPhonebookManager(m_phonebookManager);
        Request request = new Request();
        Reference reference = new Reference();
        reference.addQueryParameter("query", "searchTerm");
        request.setResourceRef(reference);
        resource.setRequest(request);
    }

    protected Collection<PhonebookEntry> getMockPhonebookEntries() {

        PhonebookEntry entry1 = createMock(PhonebookEntry.class);
        PhonebookEntry entry2 = createMock(PhonebookEntry.class);

        entry1.getFirstName();
        expectLastCall().andReturn("FirstName1");
        entry1.getLastName();
        expectLastCall().andReturn("LastName1");
        entry1.getNumber();
        expectLastCall().andReturn("200");
        entry1.getAddressBookEntry();
        expectLastCall().andReturn(new AddressBookEntry()).anyTimes();
        entry2.getFirstName();
        expectLastCall().andReturn("FirstName2");
        entry2.getLastName();
        expectLastCall().andReturn("LastName2");
        entry2.getNumber();
        expectLastCall().andReturn("201");
        entry2.getAddressBookEntry();
        expectLastCall().andReturn(new AddressBookEntry()).anyTimes();

        replay(entry1, entry2);

        return Arrays.asList(entry1, entry2);
    }

    public void testRepresentXml() throws Exception {
        Representation representation = m_resource.represent(new Variant(MediaType.TEXT_XML));

        StringWriter writer = new StringWriter();
        representation.write(writer);
        String generated = writer.toString();
        String expected = IOUtils.toString(getClass().getResourceAsStream("user-phonebook-search.rest.test.xml"));
        assertEquals(expected, generated);

        verify(m_phonebookManager);
    }

    public void testRepresentJson() throws Exception {
        Representation representation = m_resource.represent(new Variant(MediaType.APPLICATION_JSON));

        StringWriter writer = new StringWriter();
        representation.write(writer);
        String generated = writer.toString();
        String expected = IOUtils.toString(getClass().getResourceAsStream("user-phonebook-search.rest.test.json"));
        assertEquals(expected, generated);

        verify(m_phonebookManager);
    }
}
