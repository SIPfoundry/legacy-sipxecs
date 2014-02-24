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

public class PasswordResourceTest extends TestCase {

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

}
