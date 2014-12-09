/**
 * Copyright (c) 2014 eZuce, Inc. All rights reserved.
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

import java.io.IOException;
import java.util.HashMap;
import java.util.Map;

import junit.framework.TestCase;

import org.easymock.EasyMock;
import org.junit.Test;
import org.restlet.data.MediaType;
import org.restlet.data.Request;
import org.restlet.data.Response;
import org.restlet.resource.InputRepresentation;
import org.restlet.resource.ResourceException;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.permission.PermissionManager;
import org.sipfoundry.sipxconfig.security.TestAuthenticationToken;
import org.sipfoundry.sipxconfig.test.TestHelper;
import org.springframework.security.core.Authentication;
import org.springframework.security.core.context.SecurityContextHolder;

public class PasswordResourceTest extends TestCase {
    private CoreContext m_coreContext;
    private User m_user;

    @Override
    protected void setUp() throws Exception {
        m_user = new User();
        m_user.setUniqueId();
        m_user.setUserName("200");
        m_user.setFirstName("John");
        m_user.setLastName("Doe");
        m_user.setImId("JohnIM");
        m_user.setSipPassword("12345678");
        PermissionManager pManager = createMock(PermissionManager.class);
        pManager.getPermissionModel();
        expectLastCall().andReturn(TestHelper.loadSettings("commserver/user-settings.xml")).anyTimes();
        replay(pManager);
        m_user.setPermissionManager(pManager);

        Authentication token = new TestAuthenticationToken(m_user, false, false).authenticateToken();
        SecurityContextHolder.getContext().setAuthentication(token);

        m_coreContext = createMock(CoreContext.class);
        m_coreContext.loadUser(m_user.getId());
        expectLastCall().andReturn(m_user);
        replay(m_coreContext);
    }

    // can't test this, because I can't mock <br/>
    // UserDetailsImpl userDetails = StandardUserDetailsService.getUserDetails();
    @SuppressWarnings("static-method")
    public void disabledTestPasswordOk() {
        PasswordResource res = new PasswordResource();
        boolean exceptionThrown = false;
        InputRepresentation rep = new InputRepresentation(null, MediaType.TEXT_PLAIN);
        Request request = new Request();
        Map<String, Object> attributes = new HashMap<String, Object>();
        attributes.put("password", "12345678");
        request.setAttributes(attributes);
        Response response = new Response(request);
        res.init(null, request, response);

        CoreContext ctx = EasyMock.createMock(CoreContext.class);
        ctx.loadUser(EasyMock.anyInt());
        EasyMock.expectLastCall().andReturn(new User());

        try {
            res.storeRepresentation(rep);
        } catch (ResourceException expected) {
            exceptionThrown = true;
        }

        assertFalse(exceptionThrown);
    }

    @SuppressWarnings("static-method")
    @Test
    public void testPasswordTooShort() throws IOException {
        PasswordResource res = new PasswordResource();
        res.setCoreContext(m_coreContext);
        boolean exceptionThrown = false;
        InputRepresentation rep = new InputRepresentation(null, MediaType.TEXT_PLAIN);
        Request request = new Request();
        Map<String, Object> attributes = new HashMap<String, Object>();
        attributes.put("password", "123");
        request.setAttributes(attributes);
        Response response = new Response(request);
        res.init(null, request, response);

        try {
            res.storeRepresentation(rep);
        } catch (ResourceException expected) {
            exceptionThrown = true;
        }

        assertTrue(exceptionThrown);
    }

    @SuppressWarnings("static-method")
    @Test
    public void testPasswordUrlEncoded() throws IOException {
        PasswordResource res = new PasswordResource();
        res.setCoreContext(m_coreContext);
        boolean exceptionThrown = false;
        InputRepresentation rep = new InputRepresentation(null, MediaType.TEXT_PLAIN);
        Request request = new Request();
        Map<String, Object> attributes = new HashMap<String, Object>();
        attributes.put("password", "%21%24%25%5E%26%2A%28%29_%2B%7C%7E%3D%60%7B%7D%5C%5B%5C%5D%3A%3B%27%3C%3E%3F%2C.%5C%2F%40%23%5D%7B8%2C%7D");
        request.setAttributes(attributes);
        Response response = new Response(request);
        res.init(null, request, response);

        assertEquals("!$%^&*()_+|~=`{}\\[\\]:;'<>?,.\\/@#]{8,}", res.getNewPin());
    }

}
