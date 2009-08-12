/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxconfig.rest;

import java.io.InputStream;
import java.io.StringWriter;

import junit.framework.TestCase;
import org.apache.commons.io.IOUtils;
import org.restlet.data.ChallengeResponse;
import org.restlet.data.MediaType;
import org.restlet.data.Request;
import org.restlet.resource.InputRepresentation;
import org.restlet.resource.Representation;
import org.restlet.resource.Variant;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.phonebook.Address;
import org.sipfoundry.sipxconfig.phonebook.AddressBookEntry;

import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.classextension.EasyMock.createMock;

public class ContactInformationResourceTest extends TestCase {

    private User m_user;
    private CoreContext m_coreContext;

    @Override
    protected void setUp() throws Exception {
        m_user = new User();
        m_user.setUniqueId();
        m_user.setUserName("200");

        AddressBookEntry addressBook = new AddressBookEntry();
        addressBook.setJobTitle("Data Entry Assistant");
        addressBook.setJobDept("Data Management Services");
        addressBook.setCompanyName("Museum of Science");
        Address homeAddress = new Address();
        homeAddress.setCity("NY");
        addressBook.setHomeAddress(homeAddress);
        Address officeAddress = new Address();
        officeAddress.setStreet("1 Science Park");
        officeAddress.setCity("Boston");
        officeAddress.setCountry("US");
        officeAddress.setState("MA");
        officeAddress.setZip("02114");
        addressBook.setOfficeAddress(officeAddress);
        m_user.setAddressBookEntry(addressBook);

        m_coreContext = createMock(CoreContext.class);
        m_coreContext.loadUserByUserName(m_user.getUserName());
        expectLastCall().andReturn(m_user);
        m_coreContext.saveUser(m_user);
        expectLastCall().andReturn(false);
        replay(m_coreContext);
    }

    public void testRepresentXml() throws Exception {
        ContactInformationResource resource = new ContactInformationResource();
        resource.setCoreContext(m_coreContext);

        ChallengeResponse challengeResponse = new ChallengeResponse(null, "200", new char[0]);
        Request request = new Request();
        request.setChallengeResponse(challengeResponse);
        resource.init(null, request, null);

        Representation representation = resource.represent(new Variant(MediaType.TEXT_XML));
        StringWriter writer = new StringWriter();
        representation.write(writer);
        String generated = writer.toString();
        String expected = IOUtils.toString(getClass().getResourceAsStream("contact-information.rest.test.xml"));
        assertEquals(expected, generated);
    }

    public void testRepresentJson() throws Exception {
        ContactInformationResource resource = new ContactInformationResource();
        resource.setCoreContext(m_coreContext);

        ChallengeResponse challengeResponse = new ChallengeResponse(null, "200", new char[0]);
        Request request = new Request();
        request.setChallengeResponse(challengeResponse);
        resource.init(null, request, null);

        Representation representation = resource.represent(new Variant(MediaType.APPLICATION_JSON));
        StringWriter writer = new StringWriter();
        representation.write(writer);
        String generated = writer.toString();
        String expected = IOUtils.toString(getClass().getResourceAsStream("contact-information.rest.test.json"));
        assertEquals(expected, generated);
    }

    public void testStoreXml() throws Exception {
        m_user.setAddressBookEntry(new AddressBookEntry());
        ContactInformationResource resource = new ContactInformationResource();
        resource.setCoreContext(m_coreContext);
        ChallengeResponse challengeResponse = new ChallengeResponse(null, "200", new char[0]);
        Request request = new Request();
        request.setChallengeResponse(challengeResponse);
        resource.init(null, request, null);

        final InputStream xmlStream = getClass().getResourceAsStream("contact-information.rest.test.xml");
        Representation entity = new InputRepresentation(xmlStream, MediaType.TEXT_XML);
        resource.storeRepresentation(entity);

        assertEquals("Data Entry Assistant", m_user.getAddressBookEntry().getJobTitle());
        assertEquals("Data Management Services", m_user.getAddressBookEntry().getJobDept());
        assertEquals("Museum of Science", m_user.getAddressBookEntry().getCompanyName());
        assertEquals("NY", m_user.getAddressBookEntry().getHomeAddress().getCity());
        assertEquals("1 Science Park", m_user.getAddressBookEntry().getOfficeAddress().getStreet());
        assertEquals("Boston", m_user.getAddressBookEntry().getOfficeAddress().getCity());
        assertEquals("US", m_user.getAddressBookEntry().getOfficeAddress().getCountry());
        assertEquals("MA", m_user.getAddressBookEntry().getOfficeAddress().getState());
        assertEquals("02114", m_user.getAddressBookEntry().getOfficeAddress().getZip());
    }

    public void testStoreJson() throws Exception {
        m_user.setAddressBookEntry(new AddressBookEntry());
        ContactInformationResource resource = new ContactInformationResource();
        resource.setCoreContext(m_coreContext);
        ChallengeResponse challengeResponse = new ChallengeResponse(null, "200", new char[0]);
        Request request = new Request();
        request.setChallengeResponse(challengeResponse);
        resource.init(null, request, null);

        final InputStream xmlStream = getClass().getResourceAsStream("contact-information.rest.test.json");
        Representation entity = new InputRepresentation(xmlStream, MediaType.APPLICATION_JSON);
        resource.storeRepresentation(entity);

        assertEquals("Data Entry Assistant", m_user.getAddressBookEntry().getJobTitle());
        assertEquals("Data Management Services", m_user.getAddressBookEntry().getJobDept());
        assertEquals("Museum of Science", m_user.getAddressBookEntry().getCompanyName());
        assertEquals("NY", m_user.getAddressBookEntry().getHomeAddress().getCity());
        assertEquals("1 Science Park", m_user.getAddressBookEntry().getOfficeAddress().getStreet());
        assertEquals("Boston", m_user.getAddressBookEntry().getOfficeAddress().getCity());
        assertEquals("US", m_user.getAddressBookEntry().getOfficeAddress().getCountry());
        assertEquals("MA", m_user.getAddressBookEntry().getOfficeAddress().getState());
        assertEquals("02114", m_user.getAddressBookEntry().getOfficeAddress().getZip());
    }

    public void testStoreJsonEmptyUser() throws Exception {
        m_user.setAddressBookEntry(null);
        ContactInformationResource resource = new ContactInformationResource();
        resource.setCoreContext(m_coreContext);
        ChallengeResponse challengeResponse = new ChallengeResponse(null, "200", new char[0]);
        Request request = new Request();
        request.setChallengeResponse(challengeResponse);
        resource.init(null, request, null);

        final InputStream xmlStream = getClass().getResourceAsStream("contact-information.rest.test.json");
        Representation entity = new InputRepresentation(xmlStream, MediaType.APPLICATION_JSON);
        resource.storeRepresentation(entity);

        assertEquals("Data Entry Assistant", m_user.getAddressBookEntry().getJobTitle());
        assertEquals("Data Management Services", m_user.getAddressBookEntry().getJobDept());
        assertEquals("Museum of Science", m_user.getAddressBookEntry().getCompanyName());
        assertEquals("NY", m_user.getAddressBookEntry().getHomeAddress().getCity());
        assertEquals("1 Science Park", m_user.getAddressBookEntry().getOfficeAddress().getStreet());
        assertEquals("Boston", m_user.getAddressBookEntry().getOfficeAddress().getCity());
        assertEquals("US", m_user.getAddressBookEntry().getOfficeAddress().getCountry());
        assertEquals("MA", m_user.getAddressBookEntry().getOfficeAddress().getState());
        assertEquals("02114", m_user.getAddressBookEntry().getOfficeAddress().getZip());
    }
}
