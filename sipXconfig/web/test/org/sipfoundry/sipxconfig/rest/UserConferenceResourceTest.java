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
import java.util.List;

import junit.framework.TestCase;

import org.apache.commons.io.IOUtils;
import org.restlet.data.ChallengeResponse;
import org.restlet.data.MediaType;
import org.restlet.data.Request;
import org.restlet.resource.Representation;
import org.restlet.resource.Variant;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.conference.Bridge;
import org.sipfoundry.sipxconfig.conference.Conference;
import org.sipfoundry.sipxconfig.conference.ConferenceBridgeContext;
import org.sipfoundry.sipxconfig.permission.PermissionManager;
import org.sipfoundry.sipxconfig.security.TestAuthenticationToken;
import org.sipfoundry.sipxconfig.test.TestHelper;
import org.springframework.security.core.Authentication;
import org.springframework.security.core.context.SecurityContextHolder;

public class UserConferenceResourceTest extends TestCase {
    private Conference m_conference;
    private User m_user;
    private Bridge m_bridge;
    private ConferenceBridgeContext m_conferenceBridgeContext;
    private CoreContext m_coreContext;
    private UserResource m_resource;

    @Override
    protected void setUp() throws Exception {
        m_user = new User();
        m_user.setUniqueId();
        m_user.setUserName("200");
        PermissionManager pManager = createMock(PermissionManager.class);
        pManager.getPermissionModel();
        expectLastCall().andReturn(TestHelper.loadSettings("commserver/user-settings.xml")).anyTimes();
        replay(pManager);
        m_user.setPermissionManager(pManager);
        m_coreContext = createMock(CoreContext.class);
        m_coreContext.loadUserByUserName(m_user.getUserName());
        expectLastCall().andReturn(m_user);
        m_coreContext.loadUser(m_user.getId());
        expectLastCall().andReturn(m_user);
        m_coreContext.saveUser(m_user);
        expectLastCall().andReturn(false);
        replay(m_coreContext);

        Location location = new Location();
        m_bridge = new Bridge();
        m_bridge.setLocation(location);

        m_conference = new Conference();
        m_conference.setModelFilesContext(TestHelper.getModelFilesContext());
        m_conference.getSettings();
        m_conference.setEnabled(true);
        m_conference.setName("myConf");
        m_conference.setDescription("description");
        m_conference.setExtension("22123");
        m_conference.setBridge(m_bridge);
        m_conference.setSettingValue(Conference.PARTICIPANT_CODE, "555");
        m_conference.setOwner(m_user);
        List<Conference> conferences = new ArrayList<Conference>();
        conferences.add(m_conference);

        m_conferenceBridgeContext = createMock(ConferenceBridgeContext.class);
        m_conferenceBridgeContext.saveConference(m_conference);
        expectLastCall().anyTimes();
        m_conferenceBridgeContext.findConferencesByOwner(m_user);
        expectLastCall().andReturn(conferences);
        replay(m_conferenceBridgeContext);

        Authentication token = new TestAuthenticationToken(m_user, false, false).authenticateToken();
        SecurityContextHolder.getContext().setAuthentication(token);

        m_resource = new UserConferenceResource();
        UserConferenceResource resource = (UserConferenceResource) m_resource;
        resource.setConferenceBridgeContext(m_conferenceBridgeContext);
        resource.setCoreContext(m_coreContext);
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
        String expected = IOUtils.toString(getClass().getResourceAsStream("conference.rest.test.xml"));
        assertEquals(expected, generated);
    }

    public void testRepresentJson() throws Exception {
        Representation representation = m_resource.represent(new Variant(MediaType.APPLICATION_JSON));
        StringWriter writer = new StringWriter();
        representation.write(writer);
        String generated = writer.toString();
        String expected = IOUtils.toString(getClass().getResourceAsStream("conference.rest.test.json"));
        assertEquals(expected, generated);
    }
}
