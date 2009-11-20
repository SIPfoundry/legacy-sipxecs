/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.rest;

import java.io.StringWriter;
import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.Map;

import junit.framework.TestCase;
import org.acegisecurity.Authentication;
import org.acegisecurity.context.SecurityContextHolder;
import org.apache.commons.io.IOUtils;
import org.restlet.data.ChallengeResponse;
import org.restlet.data.MediaType;
import org.restlet.data.Request;
import org.restlet.resource.Representation;
import org.restlet.resource.Variant;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.phonebook.Address;
import org.sipfoundry.sipxconfig.phonebook.AddressBookEntry;
import org.sipfoundry.sipxconfig.phonebook.Phonebook;
import org.sipfoundry.sipxconfig.phonebook.PhonebookEntry;
import org.sipfoundry.sipxconfig.phonebook.PhonebookManager;
import org.sipfoundry.sipxconfig.security.TestAuthenticationToken;

import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.classextension.EasyMock.createMock;

public class UserPhonebookEntryResourceTest extends TestCase {
    private User m_user;
    private CoreContext m_coreContext;
    private Phonebook m_phonebook;
    private PhonebookManager m_phonebookManager;
    private PhonebookEntry m_entry;
    private UserResource m_resource;

    @Override
    protected void setUp() throws Exception {
        m_user = new User();
        m_user.setUniqueId();
        m_user.setUserName("200");
        m_coreContext = createMock(CoreContext.class);
        m_coreContext.loadUserByUserName(m_user.getUserName());
        expectLastCall().andReturn(m_user);
        m_coreContext.loadUser(m_user.getId());
        expectLastCall().andReturn(m_user);
        m_coreContext.saveUser(m_user);
        expectLastCall().andReturn(false);
        replay(m_coreContext);

        m_entry = new PhonebookEntry();
        m_entry.setUniqueId();
        m_entry.setFirstName("name");
        m_entry.setLastName("lastName");
        m_entry.setNumber("200");
        AddressBookEntry addressBook = new AddressBookEntry();
        addressBook.setJobTitle("title");
        addressBook.setCompanyName("company");
        Address homeAddress = new Address();
        homeAddress.setStreet("street");
        addressBook.setHomeAddress(homeAddress);
        Address officeAddress = new Address();
        officeAddress.setZip("1234");
        addressBook.setOfficeAddress(officeAddress);
        m_entry.setAddressBookEntry(addressBook);

        m_phonebook = new Phonebook();
        m_phonebook.setName("privatePhonebook_200");
        m_phonebook.setUser(m_user);
        Collection<PhonebookEntry> entries = new ArrayList<PhonebookEntry>();
        entries.add(m_entry);
        m_phonebook.setEntries(entries);
        m_phonebookManager = createMock(PhonebookManager.class);
        m_phonebookManager.getPrivatePhonebook(m_user);
        expectLastCall().andReturn(m_phonebook);
        m_phonebookManager.getPhonebookEntry(m_entry.getId());
        expectLastCall().andReturn(m_entry);
        replay(m_phonebookManager);

        Authentication token = new TestAuthenticationToken(m_user, false, false).authenticateToken();
        SecurityContextHolder.getContext().setAuthentication(token);

        m_resource = new UserPhonebookEntryResource();
        UserPhonebookEntryResource resource = (UserPhonebookEntryResource) m_resource;
        resource.setPhonebookManager(m_phonebookManager);
        resource.setCoreContext(m_coreContext);
        Request request = new Request();
        Map<String, Object> attributes = new HashMap<String, Object>();
        attributes.put("entryId", "" + m_entry.getId());
        request.setAttributes(attributes);
        ChallengeResponse challengeResponse = new ChallengeResponse(null, "200", new char[0]);
        request.setChallengeResponse(challengeResponse);
        resource.setRequest(request);
        resource.init(null, request, null);
    }

    public void testRepresentXml() throws Exception {
        Representation representation = m_resource.represent(new Variant(MediaType.TEXT_XML));
        StringWriter writer = new StringWriter();
        representation.write(writer);
        String generated = writer.toString();
        String expected = IOUtils.toString(getClass().getResourceAsStream("user-phonebook-entry.rest.test.xml"));
        assertEquals(expected, generated);
    }

    public void testRepresentJson() throws Exception {
        Representation representation = m_resource.represent(new Variant(MediaType.APPLICATION_JSON));
        StringWriter writer = new StringWriter();
        representation.write(writer);
        String generated = writer.toString();
        String expected = IOUtils.toString(getClass().getResourceAsStream("user-phonebook-entry.rest.test.json"));
        assertEquals(expected, generated);
    }
}
