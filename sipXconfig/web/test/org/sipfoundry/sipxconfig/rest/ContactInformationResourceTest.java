/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxconfig.rest;

import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.classextension.EasyMock.createMock;

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
import org.sipfoundry.commons.userdb.profile.Address;
import org.sipfoundry.commons.userdb.profile.UserProfile;
import org.sipfoundry.sipxconfig.branch.Branch;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.permission.PermissionManager;
import org.sipfoundry.sipxconfig.security.TestAuthenticationToken;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.test.TestHelper;
import org.springframework.security.core.Authentication;
import org.springframework.security.core.context.SecurityContextHolder;

public class ContactInformationResourceTest extends TestCase {

    private User m_user;
    private CoreContext m_coreContext;

    @Override
    protected void setUp() throws Exception {
        m_user = new User();
        m_user.setUniqueId();
        m_user.setUserName("200");
        m_user.setFirstName("John");
        m_user.setLastName("Doe");
        PermissionManager pManager = createMock(PermissionManager.class);
        pManager.getPermissionModel();
        Setting setting = TestHelper.loadSettings("commserver/user-settings.xml");
        setting.getSetting("im/im-account").setTypedValue(true);
        expectLastCall().andReturn(setting).anyTimes();
        replay(pManager);
        m_user.setPermissionManager(pManager);

        Authentication token = new TestAuthenticationToken(m_user, false, false).authenticateToken();
        SecurityContextHolder.getContext().setAuthentication(token);

        m_coreContext = createMock(CoreContext.class);
        m_coreContext.loadUser(m_user.getId());
        expectLastCall().andReturn(m_user);
        m_coreContext.saveUser(m_user);
        expectLastCall().andReturn(false);
        replay(m_coreContext);
    }

    @Override
    protected void tearDown() throws Exception {
        SecurityContextHolder.getContext().setAuthentication(null);
    }

    public void testRepresentXml() throws Exception {
        initUserProfile();
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
        System.out.println(generated);
        String expected = IOUtils.toString(getClass().getResourceAsStream("contact-information.rest.test.xml"));
        assertEquals(expected, generated);
    }

    public void testRepresentJson() throws Exception {
        initUserProfile();
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
        UserProfile profile = new UserProfile();
        profile.setImId("someId");
        m_user.setUserProfile(profile);
        ContactInformationResource resource = new ContactInformationResource();
        resource.setCoreContext(m_coreContext);
        ChallengeResponse challengeResponse = new ChallengeResponse(null, "200", new char[0]);
        Request request = new Request();
        request.setChallengeResponse(challengeResponse);
        resource.init(null, request, null);

        final InputStream xmlStream = getClass().getResourceAsStream("contact-information.rest.test.xml");
        Representation entity = new InputRepresentation(xmlStream, MediaType.TEXT_XML);
        resource.storeRepresentation(entity);

        assertEquals("Data Entry Assistant", m_user.getUserProfile().getJobTitle());
        assertEquals("Data Management Services", m_user.getUserProfile().getJobDept());
        assertEquals("Museum of Science", m_user.getUserProfile().getCompanyName());
        assertEquals("NY", m_user.getUserProfile().getHomeAddress().getCity());
        assertEquals("1 Science Park", m_user.getUserProfile().getOfficeAddress().getStreet());
        assertEquals("Boston", m_user.getUserProfile().getOfficeAddress().getCity());
        assertEquals("US", m_user.getUserProfile().getOfficeAddress().getCountry());
        assertEquals("MA", m_user.getUserProfile().getOfficeAddress().getState());
        assertEquals("02114", m_user.getUserProfile().getOfficeAddress().getZip());
        assertEquals("someId", m_user.getUserProfile().getImId());
    }

    public void _testRepresentXmlUserWithBranch() throws Exception {
        initUserProfile();
        Branch branch = new Branch();
        branch.getAddress().setStreet("Branch Street");
        m_user.setBranch(branch);
        m_user.getUserProfile().setUseBranchAddress(true);

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
        String expected = IOUtils.toString(getClass()
                .getResourceAsStream("contact-information-branch.rest.test.xml"));
        assertEquals(expected, generated);
    }

    public void testStoreJson() throws Exception {
        m_user.setUserProfile(new UserProfile());
        ContactInformationResource resource = new ContactInformationResource();
        resource.setCoreContext(m_coreContext);
        ChallengeResponse challengeResponse = new ChallengeResponse(null, "200", new char[0]);
        Request request = new Request();
        request.setChallengeResponse(challengeResponse);
        resource.init(null, request, null);

        final InputStream xmlStream = getClass().getResourceAsStream("contact-information.rest.test.json");
        Representation entity = new InputRepresentation(xmlStream, MediaType.APPLICATION_JSON);
        resource.storeRepresentation(entity);

        assertEquals("Data Entry Assistant", m_user.getUserProfile().getJobTitle());
        assertEquals("Data Management Services", m_user.getUserProfile().getJobDept());
        assertEquals("Museum of Science", m_user.getUserProfile().getCompanyName());
        assertEquals("NY", m_user.getUserProfile().getHomeAddress().getCity());
        assertEquals("1 Science Park", m_user.getUserProfile().getOfficeAddress().getStreet());
        assertEquals("Boston", m_user.getUserProfile().getOfficeAddress().getCity());
        assertEquals("US", m_user.getUserProfile().getOfficeAddress().getCountry());
        assertEquals("MA", m_user.getUserProfile().getOfficeAddress().getState());
        assertEquals("02114", m_user.getUserProfile().getOfficeAddress().getZip());
        assertEquals("John", m_user.getFirstName());
        assertEquals("Doe", m_user.getLastName());
    }

    public void testStoreJsonEmptyUser() throws Exception {
        ContactInformationResource resource = new ContactInformationResource();
        resource.setCoreContext(m_coreContext);
        ChallengeResponse challengeResponse = new ChallengeResponse(null, "200", new char[0]);
        Request request = new Request();
        request.setChallengeResponse(challengeResponse);
        resource.init(null, request, null);

        final InputStream xmlStream = getClass().getResourceAsStream("contact-information.rest.test.json");
        Representation entity = new InputRepresentation(xmlStream, MediaType.APPLICATION_JSON);
        resource.storeRepresentation(entity);

        assertEquals("Data Entry Assistant", m_user.getUserProfile().getJobTitle());
        assertEquals("Data Management Services", m_user.getUserProfile().getJobDept());
        assertEquals("Museum of Science", m_user.getUserProfile().getCompanyName());
        assertEquals("NY", m_user.getUserProfile().getHomeAddress().getCity());
        assertEquals("1 Science Park", m_user.getUserProfile().getOfficeAddress().getStreet());
        assertEquals("Boston", m_user.getUserProfile().getOfficeAddress().getCity());
        assertEquals("US", m_user.getUserProfile().getOfficeAddress().getCountry());
        assertEquals("MA", m_user.getUserProfile().getOfficeAddress().getState());
        assertEquals("02114", m_user.getUserProfile().getOfficeAddress().getZip());
        assertEquals("John", m_user.getFirstName());
        assertEquals("Doe", m_user.getLastName());
    }

    public void testRepresentXmlWithNullAddressBook() throws Exception {
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
        String expected = IOUtils.toString(getClass().getResourceAsStream(
                "contact-information-null-address-book.rest.test.xml"));
        assertEquals(expected, generated);
    }

    public void testRepresentJsonWithNullAddressBook() throws Exception {
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
        String expected = IOUtils.toString(getClass().getResourceAsStream(
                "contact-information-null-address-book.rest.test.json"));
        assertEquals(expected, generated);
    }

    private void initUserProfile() {
        UserProfile addressBook = new UserProfile();
        addressBook.setSalutation("Prof");
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
        addressBook.setEmailAddress("john.doe@example.com");
        addressBook.setImId("myId");
        addressBook.setTwiterName("Twitter");
        addressBook.setLinkedinName("Linkedin");
        addressBook.setFacebookName("Facebook");
        addressBook.setXingName("Xing");
        m_user.setUserProfile(addressBook);
    }
}
