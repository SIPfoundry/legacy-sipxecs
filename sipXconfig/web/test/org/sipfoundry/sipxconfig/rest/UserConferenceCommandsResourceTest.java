package org.sipfoundry.sipxconfig.rest;

import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.classextension.EasyMock.createMock;

import java.io.StringWriter;

import junit.framework.TestCase;

import org.restlet.data.ChallengeResponse;
import org.restlet.data.MediaType;
import org.restlet.data.Request;
import org.restlet.resource.Representation;
import org.restlet.resource.ResourceException;
import org.restlet.resource.Variant;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.conference.ActiveConferenceContext;
import org.sipfoundry.sipxconfig.conference.Conference;
import org.sipfoundry.sipxconfig.conference.ConferenceBridgeContext;
import org.sipfoundry.sipxconfig.permission.PermissionManager;
import org.sipfoundry.sipxconfig.security.TestAuthenticationToken;
import org.sipfoundry.sipxconfig.test.TestHelper;
import org.springframework.security.core.Authentication;
import org.springframework.security.core.context.SecurityContextHolder;

public class UserConferenceCommandsResourceTest extends TestCase {
    private User m_user;
    private ConferenceBridgeContext m_conferenceBridgeContext;
    private ActiveConferenceContext m_activeConferenceContext;
    private CoreContext m_coreContext;
    private Conference m_conference;
    private UserConferenceCommandsResource m_resource;

    private static final String TWO_200 = "200";
    private static final String FAIL_ERROR = "Should throw RESTlet resource exception";
    private static final String CONFERENCE_NAME = "portalUserConference";
    private static final String INVITATION_SENT_RESPONSE = "<command-response>Invitation sent</command-response>";
    private static final String CONFERENCE_RECORD_RESPONSE = "<command-response>Record file /tmp/freeswitch/recordings/portalUserConference.wav</command-response>";
    private static final String MUTE_PARTICIPANT_RESPONSE = "<command-response>OK mute 1\n</command-response>";
    private static final String WRONG_SET_PARAMS = "<command-response>Conference command 'wrong' not found.\n</command-response>";

    @Override
    protected void setUp() throws Exception {
        m_user = new User();
        m_user.setUniqueId();
        m_user.setUserName("portalUser");
        m_user.setEmailAddress("myName@email.com");
        PermissionManager pManager = createMock(PermissionManager.class);
        pManager.getPermissionModel();
        expectLastCall().andReturn(TestHelper.loadSettings("commserver/user-settings.xml")).anyTimes();
        replay(pManager);
        m_user.setPermissionManager(pManager);

        m_conference = new Conference();
        m_conference.setName(CONFERENCE_NAME);

        Authentication token = new TestAuthenticationToken(m_user, false, false).authenticateToken();
        SecurityContextHolder.getContext().setAuthentication(token);

        m_coreContext = createMock(CoreContext.class);
        m_coreContext.loadUser(m_user.getId());
        expectLastCall().andReturn(m_user).anyTimes();
        replay(m_coreContext);

        m_conferenceBridgeContext = createMock(ConferenceBridgeContext.class);
        m_conferenceBridgeContext.findConferenceByName(CONFERENCE_NAME);
        expectLastCall().andReturn(m_conference);
        replay(m_conferenceBridgeContext);
        m_activeConferenceContext = createMock(ActiveConferenceContext.class);

        m_resource = new UserConferenceCommandsResource();
        m_resource.setConferenceBridgeContext(m_conferenceBridgeContext);
        m_resource.setActiveConferenceContext(m_activeConferenceContext);
        m_resource.setCoreContext(m_coreContext);
    }

    public void testUserNotOwner() throws Exception {
        m_conference.setOwner(null);
        ChallengeResponse challengeResponse = new ChallengeResponse(null, TWO_200, new char[0]);
        Request request = new Request();
        request.getAttributes().put("confName", CONFERENCE_NAME);
        request.getAttributes().put("command", "xml_list");
        request.setChallengeResponse(challengeResponse);
        m_resource.init(null, request, null);

        try {
            m_resource.represent(new Variant(MediaType.TEXT_ALL));
            fail(FAIL_ERROR);
        } catch (ResourceException e) {
            // this was expected
        }
    }

    public void testInvite() throws Exception {
        m_conference.setOwner(m_user);

        ChallengeResponse challengeResponse = new ChallengeResponse(null, TWO_200, new char[0]);
        Request request = new Request();
        request.getAttributes().put("confName", CONFERENCE_NAME);
        request.getAttributes().put("command", "invite&401");
        request.setChallengeResponse(challengeResponse);
        m_resource.init(null, request, null);

        m_activeConferenceContext.inviteParticipant(m_user, m_conference, m_resource.getArguments()[1]);
        expectLastCall().once();
        replay(m_activeConferenceContext);

        String generated = getCommandResponse(m_resource);
        assertEquals(INVITATION_SENT_RESPONSE, generated);
    }

    public void testInviteIm() throws Exception {
        m_conference.setOwner(m_user);

        ChallengeResponse challengeResponse = new ChallengeResponse(null, TWO_200, new char[0]);
        Request request = new Request();
        request.getAttributes().put("confName", CONFERENCE_NAME);
        request.getAttributes().put("command", "inviteim&401");
        request.setChallengeResponse(challengeResponse);
        m_resource.init(null, request, null);

        m_activeConferenceContext.inviteImParticipant(m_user, m_conference, m_resource.getArguments()[1]);
        expectLastCall().once();
        replay(m_activeConferenceContext);

        String generated = getCommandResponse(m_resource);
        assertEquals(INVITATION_SENT_RESPONSE, generated);
    }

    private String getCommandResponse(UserConferenceCommandsResource resource) throws Exception {
        Representation representation = resource.represent(new Variant(MediaType.TEXT_ALL));
        StringWriter writer = new StringWriter();
        representation.write(writer);
        String generated = writer.toString();
        return generated;
    }

    public void testExecuteRecord() throws Exception {
        m_conference.setOwner(m_user);

        ChallengeResponse challengeResponse = new ChallengeResponse(null, TWO_200, new char[0]);
        Request request = new Request();
        request.getAttributes().put("confName", CONFERENCE_NAME);
        request.getAttributes().put("command", "record");
        request.setChallengeResponse(challengeResponse);
        m_resource.init(null, request, null);
        m_activeConferenceContext.executeCommand(m_conference, m_resource.getArguments());
        expectLastCall().andReturn(CONFERENCE_RECORD_RESPONSE).once();
        replay(m_activeConferenceContext);
        String generated = getCommandResponse(m_resource);
        assertEquals(CONFERENCE_RECORD_RESPONSE, generated);
    }

    public void testMuteParticipantResponse() throws Exception {
        m_conference.setOwner(m_user);

        ChallengeResponse challengeResponse = new ChallengeResponse(null, TWO_200, new char[0]);
        Request request = new Request();
        request.getAttributes().put("confName", CONFERENCE_NAME);
        request.getAttributes().put("command", "mute&1");
        request.setChallengeResponse(challengeResponse);
        m_resource.init(null, request, null);

        m_activeConferenceContext.executeCommand(m_conference, m_resource.getArguments());
        expectLastCall().andReturn(MUTE_PARTICIPANT_RESPONSE).once();
        replay(m_activeConferenceContext);

        String generated = getCommandResponse(m_resource);
        assertEquals(MUTE_PARTICIPANT_RESPONSE, generated);
    }

    public void testCommandError() throws Exception {
        m_conference.setOwner(m_user);

        ChallengeResponse challengeResponse = new ChallengeResponse(null, TWO_200, new char[0]);
        Request request = new Request();
        request.getAttributes().put("confName", CONFERENCE_NAME);
        request.getAttributes().put("command", "wrong&argument");
        request.setChallengeResponse(challengeResponse);
        m_resource.init(null, request, null);

        m_activeConferenceContext.executeCommand(m_conference, m_resource.getArguments());
        expectLastCall().andReturn(WRONG_SET_PARAMS).once();
        replay(m_activeConferenceContext);

        String generated = getCommandResponse(m_resource);
        assertEquals(WRONG_SET_PARAMS, generated);
    }

    public void testInviteError() throws Exception {
        m_conference.setOwner(m_user);

        ChallengeResponse challengeResponse = new ChallengeResponse(null, TWO_200, new char[0]);
        Request request = new Request();
        request.getAttributes().put("confName", CONFERENCE_NAME);
        request.getAttributes().put("command", "invite");
        request.setChallengeResponse(challengeResponse);
        m_resource.init(null, request, null);
        try {
            m_resource.represent(new Variant(MediaType.TEXT_ALL));
            fail(FAIL_ERROR);
        } catch (ResourceException e) {
            // this was expected
        }
    }
}
