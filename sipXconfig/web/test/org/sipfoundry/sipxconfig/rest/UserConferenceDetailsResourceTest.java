/**
 *
 *
 * Copyright (c) 2010 / 2012 eZuce, Inc. All rights reserved.
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

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import java.io.StringWriter;
import java.util.ArrayList;
import java.util.List;

import junit.framework.TestCase;

import org.apache.commons.io.IOUtils;
import org.restlet.data.ChallengeResponse;
import org.restlet.data.MediaType;
import org.restlet.data.Request;
import org.restlet.resource.Representation;
import org.restlet.resource.ResourceException;
import org.restlet.resource.Variant;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.conference.ActiveConference;
import org.sipfoundry.sipxconfig.conference.ActiveConferenceContext;
import org.sipfoundry.sipxconfig.conference.ActiveConferenceMember;
import org.sipfoundry.sipxconfig.conference.Conference;
import org.sipfoundry.sipxconfig.conference.ConferenceBridgeContext;
import org.sipfoundry.sipxconfig.security.TestAuthenticationToken;
import org.springframework.security.core.Authentication;
import org.springframework.security.core.context.SecurityContextHolder;

public class UserConferenceDetailsResourceTest extends TestCase {
    private User m_user;
    private ConferenceBridgeContext m_conferenceBridgeContext;
    private ActiveConferenceContext m_activeConferenceContext;
    private CoreContext m_coreContext;
    private Conference m_conference;
    private ActiveConference m_activeConference;
    private UserConferenceDetailsResource m_resource;

    private static final String CONFERENCE_NAME = "portalUserConference";
    private static final String TWO_200 = "200";
    private static final String FAIL_ERROR = "Should throw RESTlet resource exception";

    @Override
    protected void setUp() throws Exception {
        m_user = new User();
        m_user.setUniqueId();
        m_user.setUserName("portalUser");
        m_user.setEmailAddress("myName@email.com");

        m_conference = new Conference();
        m_conference.setName(CONFERENCE_NAME);
        m_conference.setExtension("555");
        m_conference.setDescription("desc");
        m_activeConference = new ActiveConference(CONFERENCE_NAME, 2, false);

        Authentication token = new TestAuthenticationToken(m_user, false, false).authenticateToken();
        SecurityContextHolder.getContext().setAuthentication(token);

        m_conferenceBridgeContext = createMock(ConferenceBridgeContext.class);
        m_conferenceBridgeContext.findConferenceByName(CONFERENCE_NAME);
        expectLastCall().andReturn(m_conference);
        replay(m_conferenceBridgeContext);
        m_activeConferenceContext = createMock(ActiveConferenceContext.class);

        m_resource = new UserConferenceDetailsResource();
        m_resource.setConferenceBridgeContext(m_conferenceBridgeContext);
        m_resource.setActiveConferenceContext(m_activeConferenceContext);
    }

    public void testUserNotOwner() throws Exception {
        m_coreContext = createMock(CoreContext.class);
        m_coreContext.loadUser(m_user.getId());
        expectLastCall().andReturn(m_user).anyTimes();
        replay(m_coreContext);
        m_resource.setCoreContext(m_coreContext);
        m_conference.setOwner(null);
        ChallengeResponse challengeResponse = new ChallengeResponse(null, TWO_200, new char[0]);
        Request request = new Request();
        request.getAttributes().put("confName", CONFERENCE_NAME);
        request.setChallengeResponse(challengeResponse);
        m_resource.init(null, request, null);

        try {
            m_resource.represent(new Variant(MediaType.TEXT_XML));
            fail(FAIL_ERROR);
        } catch (ResourceException e) {
            // this was expected
        }
    }

    public void testConferenceNotActive() throws Exception {
        m_coreContext = createMock(CoreContext.class);
        m_coreContext.loadUser(m_user.getId());
        expectLastCall().andReturn(m_user).anyTimes();
        replay(m_coreContext);
        m_resource.setCoreContext(m_coreContext);
        m_conference.setOwner(m_user);
        m_activeConferenceContext.getActiveConference(m_conference);
        expectLastCall().andReturn(null).once();
        replay(m_activeConferenceContext);

        ChallengeResponse challengeResponse = new ChallengeResponse(null, TWO_200, new char[0]);
        Request request = new Request();
        request.getAttributes().put("confName", CONFERENCE_NAME);
        request.setChallengeResponse(challengeResponse);
        m_resource.init(null, request, null);
        try {
            m_resource.represent(new Variant(MediaType.TEXT_XML));
            fail(FAIL_ERROR);
        } catch (ResourceException e) {
            // this was expected
        }
    }

    public void testConferenceDetailsXML() throws Exception {
        m_conference.setOwner(m_user);
        m_activeConference.setConference(m_conference);
        m_activeConferenceContext.getActiveConference(m_conference);
        expectLastCall().andReturn(m_activeConference).once();
        m_activeConferenceContext.getConferenceMembers(m_activeConference.getConference());
        expectLastCall().andReturn(getMembers()).once();
        replay(m_activeConferenceContext);

        m_coreContext = createMock(CoreContext.class);
        m_coreContext.loadUser(m_user.getId());
        expectLastCall().andReturn(m_user).anyTimes();
        m_coreContext.loadUserByUserNameOrAlias("200");
        expectLastCall().andReturn(getUser1()).once();
        m_coreContext.loadUserByUserNameOrAlias("201");
        expectLastCall().andReturn(getUser2()).once();
        replay(m_coreContext);
        m_resource.setCoreContext(m_coreContext);

        ChallengeResponse challengeResponse = new ChallengeResponse(null, TWO_200, new char[0]);
        Request request = new Request();
        request.getAttributes().put("confName", CONFERENCE_NAME);
        request.setChallengeResponse(challengeResponse);
        m_resource.init(null, request, null);
        Representation representation = m_resource.represent(new Variant(MediaType.TEXT_XML));
        StringWriter writer = new StringWriter();
        representation.write(writer);
        String generated = writer.toString();
        String expected = IOUtils.toString(getClass().getResourceAsStream("conferencedetails.rest.test.xml"));
        assertEquals(expected, generated);
    }

    public void testConferenceDetailsJSON() throws Exception {
        m_conference.setOwner(m_user);
        m_activeConference.setConference(m_conference);
        m_activeConferenceContext.getActiveConference(m_conference);
        expectLastCall().andReturn(m_activeConference).once();
        m_activeConferenceContext.getConferenceMembers(m_activeConference.getConference());
        expectLastCall().andReturn(getMembers()).once();
        replay(m_activeConferenceContext);

        m_coreContext = createMock(CoreContext.class);
        m_coreContext.loadUser(m_user.getId());
        expectLastCall().andReturn(m_user).anyTimes();
        m_coreContext.loadUserByUserNameOrAlias("200");
        expectLastCall().andReturn(getUser1()).once();
        m_coreContext.loadUserByUserNameOrAlias("201");
        expectLastCall().andReturn(getUser2()).once();
        replay(m_coreContext);
        m_resource.setCoreContext(m_coreContext);

        ChallengeResponse challengeResponse = new ChallengeResponse(null, TWO_200, new char[0]);
        Request request = new Request();
        request.getAttributes().put("confName", CONFERENCE_NAME);
        request.setChallengeResponse(challengeResponse);
        m_resource.init(null, request, null);
        Representation representation = m_resource.represent(new Variant(MediaType.APPLICATION_JSON));
        StringWriter writer = new StringWriter();
        representation.write(writer);
        String generated = writer.toString();
        String expected = IOUtils.toString(getClass().getResourceAsStream("conferencedetails.rest.test.json"));
        assertEquals(expected, generated);
    }

    private List<ActiveConferenceMember> getMembers() {
        List<ActiveConferenceMember> members = new ArrayList<ActiveConferenceMember>();
        ActiveConferenceMember member1 = new ActiveConferenceMember();
        member1.setCanHear(true);
        member1.setCanSpeak(true);
        member1.setEnergyLevel(30);
        member1.setId(1);
        member1.setNumber("200");
        member1.setName("200 (200@ex.example.org)");
        member1.setUuid("33ee33ee");
        member1.setVolumeIn(0);
        member1.setVolumeOut(0);
        ActiveConferenceMember member2 = new ActiveConferenceMember();
        member2.setCanHear(true);
        member2.setCanSpeak(false);
        member2.setEnergyLevel(30);
        member2.setId(1);
        member2.setNumber("201");
        member2.setName("201 (201@ex.example.org)");
        member2.setUuid("333eee333");
        member2.setVolumeIn(0);
        member2.setVolumeOut(0);
        members.add(member1);
        members.add(member2);
        return members;
    }

    private User getUser1() {
        User user1 = new User();
        user1.setUniqueId();
        user1.setUserName("200");
        user1.setImId("200im");
        return user1;
    }

    private User getUser2() {
        User user1 = new User();
        user1.setUniqueId();
        user1.setUserName("201");
        user1.setImId("201im");
        return user1;
    }
}
