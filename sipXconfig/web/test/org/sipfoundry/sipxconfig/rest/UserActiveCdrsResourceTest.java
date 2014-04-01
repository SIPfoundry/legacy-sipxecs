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
import java.util.ArrayList;
import java.util.Calendar;
import java.util.List;

import junit.framework.TestCase;

import org.apache.commons.io.IOUtils;
import org.restlet.data.ChallengeResponse;
import org.restlet.data.MediaType;
import org.restlet.data.Request;
import org.restlet.resource.Representation;
import org.restlet.resource.Variant;
import org.sipfoundry.sipxconfig.cdr.ActiveCallCdr;
import org.sipfoundry.sipxconfig.cdr.Cdr;
import org.sipfoundry.sipxconfig.cdr.CdrManager;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.permission.PermissionManager;
import org.sipfoundry.sipxconfig.security.TestAuthenticationToken;
import org.sipfoundry.sipxconfig.test.TestHelper;
import org.springframework.security.core.Authentication;
import org.springframework.security.core.context.SecurityContextHolder;

public class UserActiveCdrsResourceTest extends TestCase {
    private User m_user;
    private CoreContext m_coreContext;
    private CdrManager m_cdrManager;
    private UserResource m_resource;
    private ActiveCallCdr m_cdr;

    @Override
    protected void setUp() throws Exception {
        m_user = new User();
        m_user.setUniqueId();
        m_user.setUserName("user3");
        PermissionManager pManager = createMock(PermissionManager.class);
        pManager.getPermissionModel();
        expectLastCall().andReturn(TestHelper.loadSettings("commserver/user-settings.xml")).anyTimes();
        replay(pManager);
        m_user.setPermissionManager(pManager);

        m_cdr = new ActiveCallCdr();
        Calendar c = Calendar.getInstance();
        c.setTimeInMillis(new Long("1299785283000"));
        m_cdr.setCallerAor("<sip:user3@decebal.buc.ro:5060>");
        m_cdr.setCalleeAor("<sip:user4@decebal.buc.ro:5060>");
        m_cdr.setCalleeContact("contact");
        m_cdr.setStartTime(c.getTime());
        m_cdr.setDuration(new Long("71069032"));

        List<Cdr> cdrs = new ArrayList<Cdr>();
        cdrs.add(m_cdr);
        m_coreContext = createMock(CoreContext.class);
        m_coreContext.loadUserByUserName(m_user.getUserName());
        expectLastCall().andReturn(m_user);
        m_coreContext.loadUser(m_user.getId());
        expectLastCall().andReturn(m_user);
        m_coreContext.saveUser(m_user);
        expectLastCall().andReturn(false);

        m_cdrManager = createMock(CdrManager.class);
        m_cdrManager.getActiveCallsREST(m_user);
        expectLastCall().andReturn(cdrs);
        replay(m_coreContext, m_cdrManager);

        Authentication token = new TestAuthenticationToken(m_user, false, false).authenticateToken();
        SecurityContextHolder.getContext().setAuthentication(token);

        m_resource = new UserActiveCdrsResource();
        UserActiveCdrsResource resource = (UserActiveCdrsResource) m_resource;
        resource.setCoreContext(m_coreContext);
        resource.setCdrManager(m_cdrManager);

        ChallengeResponse challengeResponse = new ChallengeResponse(null, "200", new char[0]);
        Request request = new Request();
        request.setChallengeResponse(challengeResponse);
        resource.setRequest(request);
        resource.init(null, request, null);
    }

    public void testRepresentXml() throws Exception {
        Representation representation = m_resource.represent(new Variant(MediaType.TEXT_XML));
        StringWriter writer = new StringWriter();
        representation.write(writer);
        String generated = writer.toString();
        String expected = IOUtils.toString(getClass().getResourceAsStream("activecalls.rest.test.xml"));
        assertEquals(expected, generated);
    }

    public void testRepresentJson() throws Exception {
        Representation representation = m_resource.represent(new Variant(MediaType.APPLICATION_JSON));
        StringWriter writer = new StringWriter();
        representation.write(writer);
        String generated = writer.toString();
        String expected = IOUtils.toString(getClass().getResourceAsStream("activecalls.rest.test.json"));
        assertEquals(expected, generated);
    }
}
