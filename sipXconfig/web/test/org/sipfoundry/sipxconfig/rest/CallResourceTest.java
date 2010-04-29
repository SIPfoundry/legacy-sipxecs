package org.sipfoundry.sipxconfig.rest;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;

import org.acegisecurity.Authentication;
import org.acegisecurity.context.SecurityContextHolder;
import org.restlet.data.ChallengeResponse;
import org.restlet.data.Method;
import org.restlet.data.Request;
import org.restlet.data.Response;
import org.restlet.data.Status;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.domain.Domain;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.security.TestAuthenticationToken;
import org.sipfoundry.sipxconfig.sip.SipService;

import junit.framework.TestCase;

public class CallResourceTest extends TestCase {

    private User m_user;
    private DomainManager m_domainManager;
    private Domain m_domain;
    private CoreContext m_coreContext;
    private SipService m_sipService;

    @Override
    protected void setUp() throws Exception {
        m_user = new User();
        m_user.setUniqueId();
        m_user.setUserName("portalUser");
        m_user.setEmailAddress("myName@email.com");

        m_sipService = createMock(SipService.class);
        m_sipService.sendRefer(m_user, "sip:portalUser@example.org", "sip:123@example.org");
        replay(m_sipService);

        Authentication token = new TestAuthenticationToken(m_user, false, false).authenticateToken();
        SecurityContextHolder.getContext().setAuthentication(token);

        m_domain = new Domain();
        m_domain.setName("example.org");
        m_domainManager = createMock(DomainManager.class);
        m_domainManager.getDomain();
        expectLastCall().andReturn(m_domain).anyTimes();
        replay(m_domainManager);

        m_coreContext = createMock(CoreContext.class);
        m_coreContext.loadUser(m_user.getId());
        expectLastCall().andReturn(m_user).anyTimes();
        m_coreContext.saveUser(m_user);
        expectLastCall().andReturn(false);
        replay(m_coreContext);
    }

    public void testSipUri() throws Exception {
        CallResource resource = new CallResource();

        resource.setDomainManager(m_domainManager);
        resource.setCoreContext(m_coreContext);
        resource.setSipService(m_sipService);

        ChallengeResponse challengeResponse = new ChallengeResponse(null, "200", new char[0]);

        Request request = new Request();
        request.getAttributes().put("to", "123");
        request.setChallengeResponse(challengeResponse);
        request.setMethod(Method.PUT);
        Response response = new Response(request);

        resource.init(null, request, response);
        resource.put(null);
        Status status = response.getStatus();
        assertEquals("OK", status.getName());

        request.getAttributes().put("to", "123*we@sip");
        response = new Response(request);

        resource.init(null, request, response);
        resource.put(null);
        status = response.getStatus();
        assertEquals("Bad Request", status.getName());
    }
}
