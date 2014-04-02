package org.sipfoundry.sipxconfig.rest;

import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.classextension.EasyMock.createMock;

import java.util.ArrayList;
import java.util.List;

import junit.framework.TestCase;

import org.apache.axis.utils.ByteArrayOutputStream;
import org.apache.commons.io.IOUtils;
import org.apache.commons.lang.StringUtils;
import org.restlet.data.ChallengeResponse;
import org.restlet.data.MediaType;
import org.restlet.data.Reference;
import org.restlet.data.Request;
import org.restlet.data.Status;
import org.restlet.resource.Representation;
import org.restlet.resource.ResourceException;
import org.restlet.resource.Variant;
import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.ivr.Ivr;
import org.sipfoundry.sipxconfig.permission.PermissionManager;
import org.sipfoundry.sipxconfig.rest.RestRedirectorResource.HttpInvoker;
import org.sipfoundry.sipxconfig.restserver.RestServer;
import org.sipfoundry.sipxconfig.security.TestAuthenticationToken;
import org.sipfoundry.sipxconfig.test.TestHelper;
import org.sipfoundry.sipxconfig.vm.MailboxManager;
import org.springframework.security.core.Authentication;
import org.springframework.security.core.context.SecurityContextHolder;

public class RestRedirectorResourceTest extends TestCase {
    private User m_user;
    private CoreContext m_coreContext;
    private AddressManager m_addressManager;
    private MailboxManager m_mailboxManager;

    @Override
    protected void setUp() throws Exception {
        m_user = new User();
        m_user.setUniqueId();
        m_user.setUserName("200");
        m_user.setFirstName("John");
        m_user.setLastName("Doe");
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

        m_addressManager = createMock(AddressManager.class);
        m_addressManager.getSingleAddress(RestServer.HTTP_API);
        expectLastCall().andReturn(new Address(RestServer.HTTP_API, "host.example.com", 6667));
        m_addressManager.getAddresses(Ivr.REST_API);
        Address ivrAddress = new Address(Ivr.REST_API, "host.example.com", 8085);
        List<Address> addresses = new ArrayList<Address>();
        addresses.add(ivrAddress);
        expectLastCall().andReturn(addresses);
        replay(m_addressManager);

        m_mailboxManager = createMock(MailboxManager.class);
        m_mailboxManager.getLastGoodIvrNode();
        expectLastCall().andReturn(null);
        m_mailboxManager.setLastGoodIvrNode(ivrAddress);
        expectLastCall().anyTimes();
        replay(m_mailboxManager);
    }

    public void testRepresentIvr() throws Exception {
        represent("http://host.example.com:8085", "http://host.example.com:8085/mailbox/200/messages", RestRedirectorResource.MAILBOX, "<messages></messages>".getBytes());
    }

    public void testRepresentCdr() throws Exception {
        represent("http://host.example.com:6667", "http://host.example.com:6666/cdr/200", RestRedirectorResource.CDR, "<cdr></cdr>".getBytes());
    }

    public void testRepresentNothing() throws Exception {
        genericNotFoundTest("");
    }

    public void testRepresentUnknown() throws Exception {
        genericNotFoundTest("/idonotexist");
    }

    private void genericNotFoundTest(String res) throws ResourceException {
        HttpInvoker invoker = createMock(HttpInvoker.class);
        RestRedirectorResource resource = createResource(invoker, "");
        invoker.invokeGet("http://host.example.com/my/redirect" + res);
        Object unimportantRespones = new byte[0];
        expectLastCall().andReturn(unimportantRespones).once();
        replay(invoker);

        Status reStatus = null;
        try {
            resource.represent(new Variant(MediaType.ALL));
        } catch (ResourceException re) {
            reStatus = re.getStatus();
        }

        assertEquals(Status.CLIENT_ERROR_NOT_FOUND, reStatus);
    }

    public void testPost() throws Exception {
        post("http://host.example.com:6667", "http://host.example.com:6666/callcontroller/200/201", RestRedirectorResource.CALLCONTROLLER);
    }

    public void testPut() throws Exception {
        put("http://host.example.com:8085", "http://host.example.com:8085/mailbox/200/message/0000001/heard", RestRedirectorResource.MAILBOX);
    }

    public void testDelete() throws Exception {
        delete("http://host.example.com:8085", "http://host.example.com:8085/mailbox/200/message/0000001/heard", RestRedirectorResource.MAILBOX);
    }

    private void represent(String address, String resIdentifier, String resourceType, byte[] result) throws Exception{
        HttpInvoker invoker = createMock(HttpInvoker.class);
        String uri = StringUtils.substringAfter(resIdentifier, resourceType);
        invoker.invokeGet(address + resourceType + uri);
        expectLastCall().andReturn(result).once();
        replay(invoker);

        RestRedirectorResource resource = createResource(invoker, resIdentifier);

        Representation representation = resource.represent(new Variant(MediaType.ALL));
        ByteArrayOutputStream writer = null;
        try {
            writer = new ByteArrayOutputStream();
            representation.write(writer);
            byte[] generated = writer.toByteArray();

            assertEquals(new String(result), new String(generated));
        } finally {
            IOUtils.closeQuietly(writer);
        }
    }

    private void post(String address, String resIdentifier, String resourceType) throws Exception{
        HttpInvoker invoker = createMock(HttpInvoker.class);
        String uri = StringUtils.substringAfter(resIdentifier, resourceType);
        invoker.invokePost(address + resourceType + uri);
        expectLastCall().once();
        replay(invoker);

        RestRedirectorResource resource = createResource(invoker, resIdentifier);

        resource.acceptRepresentation(null);
    }

    private void put(String address, String resIdentifier, String resourceType) throws Exception{
        HttpInvoker invoker = createMock(HttpInvoker.class);
        String uri = StringUtils.substringAfter(resIdentifier, resourceType);
        invoker.invokePut(address + resourceType + uri);
        expectLastCall().once();
        replay(invoker);

        RestRedirectorResource resource = createResource(invoker, resIdentifier);

        resource.storeRepresentation(null);
    }

    private void delete(String address, String resIdentifier, String resourceType) throws Exception{
        HttpInvoker invoker = createMock(HttpInvoker.class);
        String uri = StringUtils.substringAfter(resIdentifier, resourceType);
        invoker.invokeDelete(address + resourceType + uri);
        expectLastCall().once();
        replay(invoker);

        RestRedirectorResource resource = createResource(invoker, resIdentifier);

        resource.removeRepresentations();
    }

    private RestRedirectorResource createResource(HttpInvoker invoker, String resIdentifier) {
        RestRedirectorResource resource = new RestRedirectorResource();
        resource.setCoreContext(m_coreContext);
        resource.setAddressManager(m_addressManager);
        resource.setMailboxManager(m_mailboxManager);
        resource.setHttpInvoker(invoker);

        ChallengeResponse challengeResponse = new ChallengeResponse(null, "200", new char[0]);
        Request request = new Request();
        Reference resourceRef = new Reference();
        resourceRef.setIdentifier(resIdentifier);
        request.setResourceRef(resourceRef);
        request.setChallengeResponse(challengeResponse);
        resource.init(null, request, null);

        return resource;
    }
}
