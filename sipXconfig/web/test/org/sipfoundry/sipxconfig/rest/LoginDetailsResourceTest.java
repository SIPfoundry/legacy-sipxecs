/**
 *
 *
 * Copyright (c) 2010 / 2011 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.rest;

import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.classextension.EasyMock.createMock;

import java.io.StringWriter;

import junit.framework.TestCase;

import org.apache.commons.io.IOUtils;
import org.restlet.data.ChallengeResponse;
import org.restlet.data.MediaType;
import org.restlet.data.Request;
import org.restlet.resource.Representation;
import org.restlet.resource.Variant;
import org.sipfoundry.sipxconfig.bulk.ldap.LdapManager;
import org.sipfoundry.sipxconfig.bulk.ldap.LdapSystemSettings;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.security.TestAuthenticationToken;
import org.springframework.security.core.Authentication;
import org.springframework.security.core.context.SecurityContextHolder;

public class LoginDetailsResourceTest extends TestCase {

    private User m_user;
    private CoreContext m_coreContext;
    private LdapManager m_ldapManager;

    @Override
    protected void setUp() throws Exception {
        m_user = new User();
        m_user.setUniqueId();
        m_user.setUserName("200");
        m_user.setFirstName("John");
        m_user.setLastName("Doe");
        m_user.setImId("JohnIM");
        m_user.setSipPassword("12345678");

        Authentication token = new TestAuthenticationToken(m_user, false, false).authenticateToken();
        SecurityContextHolder.getContext().setAuthentication(token);

        m_coreContext = createMock(CoreContext.class);
        m_coreContext.loadUser(m_user.getId());
        expectLastCall().andReturn(m_user);
        m_ldapManager = createMock(LdapManager.class);
        LdapSystemSettings settings = new LdapSystemSettings();
        settings.setEnableOpenfireConfiguration(false);
        m_ldapManager.getSystemSettings();
        expectLastCall().andReturn(settings).once();
        replay(m_coreContext, m_ldapManager);
    }

    @Override
    protected void tearDown() throws Exception {
        SecurityContextHolder.getContext().setAuthentication(null);
    }

    public void testRepresentXml() throws Exception {
        LoginDetailsResource resource = new LoginDetailsResource();
        resource.setCoreContext(m_coreContext);
        resource.setLdapManager(m_ldapManager);

        ChallengeResponse challengeResponse = new ChallengeResponse(null, "200", new char[0]);
        Request request = new Request();
        request.setChallengeResponse(challengeResponse);
        resource.init(null, request, null);

        Representation representation = resource.represent(new Variant(MediaType.TEXT_XML));
        StringWriter writer = new StringWriter();
        representation.write(writer);
        String generated = writer.toString();
        String expected = IOUtils.toString(getClass().getResourceAsStream("logindetails.rest.test.xml"));
        assertEquals(expected, generated);
    }

    public void testRepresentJson() throws Exception {
        LoginDetailsResource resource = new LoginDetailsResource();
        resource.setCoreContext(m_coreContext);
        resource.setLdapManager(m_ldapManager);

        ChallengeResponse challengeResponse = new ChallengeResponse(null, "200", new char[0]);
        Request request = new Request();
        request.setChallengeResponse(challengeResponse);
        resource.init(null, request, null);

        Representation representation = resource.represent(new Variant(MediaType.APPLICATION_JSON));
        StringWriter writer = new StringWriter();
        representation.write(writer);
        String generated = writer.toString();
        String expected = IOUtils.toString(getClass().getResourceAsStream("logindetails.rest.test.json"));
        assertEquals(expected, generated);
    }

}
