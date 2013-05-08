/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.rest;

import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.classextension.EasyMock.createMock;

import java.io.IOException;
import java.io.OutputStream;
import java.io.StringWriter;
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
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.device.AbstractProfileGenerator;
import org.sipfoundry.sipxconfig.device.ProfileContext;
import org.sipfoundry.sipxconfig.device.ProfileGenerator;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.LineInfo;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.phone.PhoneModel;
import org.sipfoundry.sipxconfig.security.TestAuthenticationToken;
import org.sipfoundry.sipxconfig.test.MemoryProfileLocation;

public class ProfileResourceTest extends TestCase {
    private User m_user;
    private Phone m_phone;
    private PhoneContext m_phoneContext;
    private ProfileResource m_resource;

    @Override
    protected void setUp() throws Exception {
        m_user = new User();
        m_user.setUniqueId();
        m_user.setUserName("200");
        m_user.setFirstName("John");
        m_user.setLastName("Doe");

        Authentication token = new TestAuthenticationToken(m_user, false, false).authenticateToken();
        SecurityContextHolder.getContext().setAuthentication(token);

        m_phoneContext = createMock(PhoneContext.class);
        m_phone = new LimitedPhone();
        m_phone.setSerialNumber("123456");
        m_phone.getModel().setDefaultProfileLocation(new MemoryProfileLocation());

        m_phoneContext.getPhoneIdBySerialNumber(m_phone.getSerialNumber());
        Integer phoneId = 1;
        expectLastCall().andReturn(phoneId);
        m_phoneContext.loadPhone(phoneId);
        expectLastCall().andReturn(m_phone);
        replay(m_phoneContext);

        m_resource = new ProfileResource();
        m_resource.setPhoneContext(m_phoneContext);

        Request request = new Request();
        Map<String, Object> attributes = new HashMap<String, Object>();
        attributes.put("serialNumber", "123456");
        attributes.put("name", "profile.rest.test.ini");
        request.setAttributes(attributes);

        ChallengeResponse challengeResponse = new ChallengeResponse(null, "200", new char[0]);
        request.setChallengeResponse(challengeResponse);
        m_resource.setRequest(request);
        m_resource.init(null, request, null);
    }

    @Override
    protected void tearDown() throws Exception {
        SecurityContextHolder.getContext().setAuthentication(null);
    }

    public void testRepresent() throws Exception {

        Representation representation = m_resource.represent(new Variant(MediaType.APPLICATION_ALL));
        StringWriter writer = new StringWriter();
        representation.write(writer);

        String generated = writer.toString();
        String expected = IOUtils.toString(getClass().getResourceAsStream("profile.rest.test.ini"));
        assertEquals(generated, expected);
    }

    static class LimitedPhone extends Phone {
        LimitedPhone() {
            setModel(new PhoneModel());
        }

        @Override
        public void setLineInfo(Line line, LineInfo externalLine) {
        }

        @Override
        public LineInfo getLineInfo(Line line) {
            return null;
        }

        @Override
        public ProfileGenerator getProfileGenerator() {
            return new LimitedProfileGenerator();
        }

        @Override
        public String getProfileFilename() {
            return "profile.rest.test.ini";
        }

    }

    static class LimitedProfileGenerator extends AbstractProfileGenerator {

        @Override
        protected void generateProfile(ProfileContext context, OutputStream out) throws IOException {
            Phone phone = (Phone) context.getDevice();
            String content = IOUtils.toString(getClass().getResourceAsStream(phone.getProfileFilename()));
            out.write(content.getBytes());
        }

    }
}
