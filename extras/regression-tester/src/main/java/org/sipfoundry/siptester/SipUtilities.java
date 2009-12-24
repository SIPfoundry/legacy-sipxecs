package org.sipfoundry.siptester;

import gov.nist.javax.sip.ListeningPointExt;
import gov.nist.javax.sip.SipStackExt;
import gov.nist.javax.sip.address.SipURIExt;
import gov.nist.javax.sip.header.HeaderFactoryExt;
import gov.nist.javax.sip.header.HeaderFactoryImpl;
import gov.nist.javax.sip.header.extensions.ReferredByHeader;
import gov.nist.javax.sip.message.MessageFactoryExt;
import gov.nist.javax.sip.message.RequestExt;
import gov.nist.javax.sip.message.ResponseExt;

import java.net.InetAddress;
import java.text.ParseException;
import java.util.Collection;
import java.util.Collections;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.ListIterator;
import java.util.Random;

import javax.sdp.Origin;
import javax.sdp.SdpException;
import javax.sdp.SdpFactory;
import javax.sdp.SdpParseException;
import javax.sdp.SessionDescription;
import javax.sip.ClientTransaction;
import javax.sip.Dialog;
import javax.sip.DialogState;
import javax.sip.InvalidArgumentException;
import javax.sip.ListeningPoint;
import javax.sip.ServerTransaction;
import javax.sip.SipException;
import javax.sip.SipProvider;
import javax.sip.TransactionAlreadyExistsException;
import javax.sip.TransactionUnavailableException;
import javax.sip.address.Address;
import javax.sip.address.AddressFactory;
import javax.sip.address.Hop;
import javax.sip.address.SipURI;
import javax.sip.header.AcceptHeader;
import javax.sip.header.AllowHeader;
import javax.sip.header.CSeqHeader;
import javax.sip.header.CallIdHeader;
import javax.sip.header.ContactHeader;
import javax.sip.header.ContentTypeHeader;
import javax.sip.header.EventHeader;
import javax.sip.header.ExpiresHeader;
import javax.sip.header.FromHeader;
import javax.sip.header.Header;
import javax.sip.header.HeaderFactory;
import javax.sip.header.MaxForwardsHeader;
import javax.sip.header.ReasonHeader;
import javax.sip.header.ReferToHeader;
import javax.sip.header.RouteHeader;
import javax.sip.header.ToHeader;
import javax.sip.header.ViaHeader;
import javax.sip.message.Message;
import javax.sip.message.MessageFactory;
import javax.sip.message.Request;
import javax.sip.message.Response;

import org.apache.log4j.Logger;
import org.sipfoundry.commons.siprouter.FindSipServer;
import org.sipfoundry.commons.userdb.User;
import org.sipfoundry.commons.userdb.ValidUsersXML;

public class SipUtilities {
    private static final Logger logger = Logger.getLogger(SipUtilities.class);

    private static final String APPLICATION = "application";

    private static final String SDP = "sdp";

    private int m_maxForwards = 70;

    private static HeaderFactoryExt headerFactory = (HeaderFactoryExt) SipTester.getStackBean()
            .getHeaderFactory();

    private static MessageFactoryExt messageFactory = (MessageFactoryExt) SipTester
            .getStackBean().getMessageFactory();

    private static AddressFactory addressFactory = (AddressFactory) SipTester.getStackBean()
            .getAddressFactory();

    public final FromHeader createFromHeader(String fromDisplayName, SipURI fromAddress)
            throws ParseException {
        Address fromNameAddress = addressFactory.createAddress(fromDisplayName, fromAddress);
        return headerFactory.createFromHeader(fromNameAddress, Integer.toString(Math
                .abs(new Random().nextInt())));
    }

    final public static ContactHeader createContactHeader(ListeningPointExt listeningPoint)
            throws ParseException {
        ContactHeader contactHeader = listeningPoint.createContactHeader();
        Address address = contactHeader.getAddress();
        SipURI sipUri = (SipURI) address.getURI();

        sipUri.setUser("sipxtester");

        return contactHeader;
    }

    static final public long getSequenceNumber(Message sipMessage) {
        CSeqHeader cseqHeader = (CSeqHeader) sipMessage.getHeader(CSeqHeader.NAME);
        return cseqHeader.getSeqNumber();
    }

    final public ToHeader createToHeader(SipURI toURI) throws ParseException {
        Address toAddress = SipTester.getStackBean().getAddressFactory().createAddress(toURI);
        return headerFactory.createToHeader(toAddress, null);
    }

    final public AcceptHeader createAcceptHeader(String type, String subType)
            throws ParseException {
        return headerFactory.createAcceptHeader(type, subType);
    }

    final public static ViaHeader createViaHeader(ListeningPointExt listeningPoint)
            throws ParseException, InvalidArgumentException {
        return listeningPoint.createViaHeader();
    }

    public final ReasonHeader createReasonHeader(String reason) throws ParseException,
            InvalidArgumentException {
        ReasonHeader reasonHeader = headerFactory.createReasonHeader("sipxtester", 1024, reason);
        return reasonHeader;
    }

    final public void addContent(Request request, String contentType, byte[] payload)
            throws ParseException {
        if (contentType == null) {
            return;
        }
        String[] ct = contentType.split("/", 2);
        ContentTypeHeader contentTypeHeader = headerFactory.createContentTypeHeader(ct[0], ct[1]);
        if (contentTypeHeader != null) {
            request.setContent(payload, contentTypeHeader);
        }
    }

    final public void addEventHeader(Request request, String eventType) throws ParseException {
        EventHeader eventHeader = headerFactory.createEventHeader(eventType);
        request.addHeader(eventHeader);
    }

    final public void addHeader(Request request, String name, String value) throws ParseException {
        Header header = headerFactory.createHeader(name, value);
        request.addHeader(header);
    }

    final public void setContent(Message message, SessionDescription sessionDescription) {
        try {
            ContentTypeHeader cth = headerFactory.createContentTypeHeader(APPLICATION, SDP);
            String sd = sessionDescription.toString();
            message.setContent(sd, cth);
        } catch (ParseException ex) {
            throw new SipTesterException(ex);
        }

    }

    final public Response createResponse(Request request, int responseCode) throws ParseException {
        Response response = messageFactory.createResponse(responseCode, request);
        return response;
    }

    final public static String getCSeqMethod(Message response) {
        return ((CSeqHeader) response.getHeader(CSeqHeader.NAME)).getMethod();
    }

    final public void setSdpContent(Message message, String sdpContent) {
        try {
            ContentTypeHeader cth = this.createContentTypeHeader();
            message.setContent(sdpContent, cth);
        } catch (Exception ex) {
            logger.error("Unexpected exception creating header", ex);
            throw new SipTesterException(ex);
        }
    }

    final public ReferToHeader createReferToHeader(String referToAddrSpec) {
        try {
            String referToUri = "sip:" + referToAddrSpec;
            SipURI sipUri = (SipURI) addressFactory.createURI(referToUri);
            Address address = addressFactory.createAddress(sipUri);
            ReferToHeader referToHeader = headerFactory.createReferToHeader(address);
            return referToHeader;
        } catch (ParseException ex) {
            logger.error("Unexpected exception creating header", ex);
            throw new SipTesterException(ex);
        }
    }

    final public ContentTypeHeader createContentTypeHeader() {
        try {
            return headerFactory.createContentTypeHeader(APPLICATION, SDP);
        } catch (Exception ex) {
            logger.error("Unexpected exception creating header", ex);
            throw new SipTesterException(ex);
        }
    }

    final public ReferredByHeader createReferredByHeader(String addrSpec) throws ParseException {
        String referredByUri = "sip:" + addrSpec;
        Address address = addressFactory.createAddress(addressFactory.createURI(referredByUri));
        return ((HeaderFactoryImpl) headerFactory).createReferredByHeader(address);
    }

    final public String getSipDomain(String uri) throws Exception {
        SipURI sipUri = (SipURI) addressFactory.createURI(uri);
        return sipUri.getHost();
    }

    public static String getToTag(Message message) {
        return ((ToHeader) message.getHeader(ToHeader.NAME)).getTag();
    }

    public static String getToAddress(Message message) {
        return ((ToHeader) message.getHeader(ToHeader.NAME)).getAddress().toString();
    }

    public static String getToAddrSpec(Message message) {
        String user = ((SipURI) ((ToHeader) message.getHeader(ToHeader.NAME)).getAddress()
                .getURI()).getUser();
        String host = ((SipURI) ((ToHeader) message.getHeader(ToHeader.NAME)).getAddress()
                .getURI()).getHost();
        return user + "@" + host;
    }

    public static String getCallId(Message message) {
        return ((CallIdHeader) message.getHeader(CallIdHeader.NAME)).getCallId();
    }

    public static String getFromTag(Message message) {
        return ((FromHeader) message.getHeader(FromHeader.NAME)).getTag();
    }

    public static String getFromAddress(Message message) {
        return ((FromHeader) message.getHeader(FromHeader.NAME)).getAddress().toString();
    }

    public static String getFromAddrSpec(Message message) {
        String user = ((SipURI) ((FromHeader) message.getHeader(FromHeader.NAME)).getAddress()
                .getURI()).getUser();
        String host = ((SipURI) ((FromHeader) message.getHeader(FromHeader.NAME)).getAddress()
                .getURI()).getHost();
        return user + "@" + host;
    }

    public static String getFromDisplayName(Message message) {
        return ((FromHeader) message.getHeader(FromHeader.NAME)).getAddress().getDisplayName();
    }

    public static String getFromUserName(Message message) {
        return ((SipURI) ((FromHeader) message.getHeader(FromHeader.NAME)).getAddress().getURI())
                .getUser();
    }

    public void addSdpContent(Message message, String sdpContent) throws SipTesterException {
        try {
            ContentTypeHeader cth = headerFactory.createContentTypeHeader("application", "sdp");
            message.setContent(sdpContent, cth);
        } catch (Exception ex) {
            throw new SipTesterException(ex);
        }
    }

    public static SessionDescription getSessionDescription(Message message) {
        if (message.getRawContent() == null)
            throw new SipTesterException(new SdpParseException(0, 0, "Missing sdp body"));
        try {
            String messageString = new String(message.getRawContent());
            SessionDescription sd = SdpFactory.getInstance().createSessionDescription(
                    messageString);
            return sd;
        } catch (SdpParseException ex) {
            throw new SipTesterException(ex);
        }
    }

    public static SessionDescription incrementSessionDescriptionVersionNumber(Response response) {
        SessionDescription sd = getSessionDescription(response);
        try {
            long versionNumber = sd.getOrigin().getSessionVersion();
            SdpFactory sdpFactory = SdpFactory.getInstance();

            SessionDescription newSd = sdpFactory.createSessionDescription(sd.toString());
            Origin origin = newSd.getOrigin();
            origin.setSessionVersion(versionNumber + 1);
            return newSd;
        } catch (SdpParseException ex) {
            throw new SipTesterException(ex);
        } catch (SdpException ex) {
            throw new SipTesterException(ex);
        }

    }

    public static SessionDescription incrementSessionDescriptionVersionNumber(
            SessionDescription sd) {
        try {
            long versionNumber = sd.getOrigin().getSessionVersion();
            SdpFactory sdpFactory = SdpFactory.getInstance();

            SessionDescription newSd = sdpFactory.createSessionDescription(sd.toString());
            Origin origin = newSd.getOrigin();
            origin.setSessionVersion(versionNumber + 1);
            return newSd;
        } catch (SdpParseException ex) {
            throw new SipTesterException(ex);
        } catch (SdpException ex) {
            throw new SipTesterException(ex);
        }

    }

    public static SessionDescription decrementSessionDescriptionVersionNumber(Response response) {
        SessionDescription sd = getSessionDescription(response);
        try {
            long versionNumber = sd.getOrigin().getSessionVersion();
            SdpFactory sdpFactory = SdpFactory.getInstance();

            SessionDescription newSd = sdpFactory.createSessionDescription(sd.toString());
            Origin origin = newSd.getOrigin();
            origin.setSessionVersion(versionNumber - 1);
            return newSd;
        } catch (SdpParseException ex) {
            throw new SipTesterException(ex);
        } catch (SdpException ex) {
            throw new SipTesterException(ex);
        }
    }

    /**
     * Create a basic registration request.
     */

    static Request createRegistrationRequestTemplate(String userName, Endpoint endpoint,
            SipProvider sipProvider, String callId, long cseq) throws ParseException,
            InvalidArgumentException, SipException {
        AddressFactory addressFactory = endpoint.getStackBean().getAddressFactory();

        HeaderFactory headerFactory = endpoint.getStackBean().getHeaderFactory();

        String registrar = SipTester.getTesterConfig().getSipxProxyDomain();

        SipURI requestUri = addressFactory.createSipURI(null, registrar);

        if (sipProvider == null)
            throw new NullPointerException("Null sipProvider");

        /*
         * We register with From and To headers set to the proxy domain.
         */
        String proxyDomain = SipTester.getTesterConfig().getSipxProxyDomain();
        SipURI fromUri = SipTester.getStackBean().getAddressFactory().createSipURI(userName,
                proxyDomain);

        SipURI toUri = addressFactory.createSipURI(userName, proxyDomain);

        Address fromAddress = addressFactory.createAddress(fromUri);

        FromHeader fromHeader = headerFactory.createFromHeader(fromAddress, new Long(Math
                .abs(new java.util.Random().nextLong())).toString());

        Address toAddress = addressFactory.createAddress(toUri);

        ToHeader toHeader = headerFactory.createToHeader(toAddress, null);

        CallIdHeader callidHeader = callId == null ? sipProvider.getNewCallId() : headerFactory
                .createCallIdHeader(callId);

        CSeqHeader cseqHeader = headerFactory.createCSeqHeader(cseq, Request.REGISTER);

        MaxForwardsHeader maxForwards = headerFactory.createMaxForwardsHeader(20);

        ViaHeader viaHeader = createViaHeader(endpoint.getDefaultListeningPoint());

        List<ViaHeader> list = new LinkedList<ViaHeader>();
        list.add(viaHeader);

        Request request = messageFactory.createRequest(requestUri, Request.REGISTER,
                callidHeader, cseqHeader, fromHeader, toHeader, list, maxForwards);

        String outboundRegistrar = registrar;

        SipURI registrarUri = addressFactory.createSipURI(null, outboundRegistrar);

        Collection<Hop> hops = new FindSipServer(logger).findSipServers(registrarUri);

        if (hops == null || hops.isEmpty()) {
            throw new SipException("No route to registrar found");
        }

        RouteHeader routeHeader = SipUtilities.createRouteHeader(hops.iterator().next());
        request.setHeader(routeHeader);
        return request;
    }

    static Request createRegistrationRequest(SipProvider sipProvider, String userName,
            Endpoint endpoint, String callId, long cseq) throws SipException, SipTesterException {

        try {
            Request request = createRegistrationRequestTemplate(userName, endpoint, sipProvider,
                    callId, cseq);

            ContactHeader contactHeader = createContactHeader(endpoint);
            contactHeader.removeParameter("expires");

            request.addHeader(contactHeader);
            int registrationTimer = 600;
            ExpiresHeader expiresHeader = SipTester.getHeaderFactory().createExpiresHeader(
                    registrationTimer);
            request.addHeader(expiresHeader);
            return request;
        } catch (Exception ex) {
            String s = "Unexpected error creating register -- check proxy configuration ";
            logger.error(s, ex);
            throw new SipTesterException(s, ex);

        }

    }

    private static ContactHeader createContactHeader(Endpoint endpoint) {
        ListeningPointExt listeningPoint = endpoint.getDefaultListeningPoint();
        return listeningPoint.createContactHeader();
    }

    public static RouteHeader createRouteHeader(Hop hop) {
        try {
            SipURI routeUri = SipTester.getAddressFactory().createSipURI(null,
                    InetAddress.getByName(hop.getHost()).getHostAddress());
            if (hop.getPort() != -1) {
                routeUri.setPort(hop.getPort());
            }
            routeUri.setTransportParam(hop.getTransport());
            routeUri.setLrParam();
            Address routeAddress = SipTester.getAddressFactory().createAddress(routeUri);
            RouteHeader routeHeader = SipTester.getHeaderFactory()
                    .createRouteHeader(routeAddress);
            return routeHeader;
        } catch (Exception ex) {
            String s = "Unexpected exception";
            logger.fatal(s, ex);
            throw new SipTesterException(s, ex);
        }
    }

    public static Address remapAddress(Address oldAddress) {
        try {
            SipURI sipUri = (SipURI) oldAddress.getURI();

            if (sipUri.getHost().equals(SipTester.getSutDomainName())) {
                String user = sipUri.getUser();
                String testUser = SipTester.getTestUser(user);
                SipURI newSipUri = SipTester.getAddressFactory().createSipURI(testUser,
                        SipTester.getTesterConfig().getSipxProxyDomain());
                Iterator<String> names = sipUri.getParameterNames();
                while (names.hasNext()) {
                    String name = names.next();
                    String value = sipUri.getParameter(name);
                    newSipUri.setHeader(name, value);
                }
                return SipTester.getAddressFactory().createAddress(newSipUri);
            } else {
                return oldAddress;
            }
        } catch (Exception ex) {
            SipTester.fail("Unexpected exception",ex);
            throw new SipTesterException(ex);
        }
    }

    /**
     * This routine copies headers from inbound to outbound responses.
     * 
     * @param message
     * @param newMessage
     */
    public static void copyHeaders(Message message, Message newMessage) {
        Iterator<String> headerNames = message.getHeaderNames();
        while (headerNames.hasNext()) {
            String headerName = headerNames.next();
            if (newMessage.getHeader(headerName) == null) {
                ListIterator<Header> headerIterator = message.getHeaders(headerName);
                while (headerIterator.hasNext()) {
                    Header header = headerIterator.next();
                    Header newHeader = header;
                    if (newHeader.getName().equals(ReferToHeader.NAME)) {
                        ReferToHeader referToHeader = (ReferToHeader) newHeader;
                        Address address = referToHeader.getAddress();
                        Address newAddress = remapAddress(address);
                        newHeader = SipTester.getHeaderFactory().createReferToHeader(newAddress);
                    } else if (newHeader.getName().equals(ReferredByHeader.NAME)) {
                        ReferredByHeader referToHeader = (ReferredByHeader) newHeader;
                        Address address = referToHeader.getAddress();
                        Address newAddress = remapAddress(address);
                        newHeader = ((HeaderFactoryExt)SipTester.getHeaderFactory()).createReferredByHeader(newAddress);
                   
                    }
                    newMessage.addHeader(newHeader);
                }
            }
        }

    }

    /**
     * Creates an emulation INVITE.
     * 
     * @param sipRequest - captured INVITE.
     * 
     * @return emulated INVITE or null
     */
    public static RequestExt createInviteRequest(RequestExt sipRequest, Endpoint endpoint)
            throws Exception {
        SipURI sipUri = (SipURI) sipRequest.getRequestURI();
        String toUser = ((SipURI) sipRequest.getToHeader().getAddress().getURI()).getUser();
        ValidUsersXML validUsers = SipTester.getSutValidUsers();
        String domain = sipUri.getHost();
        String method = sipRequest.getMethod();
        /*
         * Find the UserAgent that corresponds to the destination of the INVITE.
         */
        SutUA targetTestUa = null;
        String targetUser = null;
        for (User user : validUsers.GetUsers()) {
            if ((toUser.equals(user.getUserName()) || user.getAliases().contains(toUser))) {
                targetTestUa = SipTester.getSutUA(user.getUserName());
                targetUser = SipTester.getTestUser(user.getUserName());
                break;
            }
        }
        logger.debug("found To valid user " + targetUser);

        /*
         * If we found a mapped user agent then rewrite that request. Otherwise this is possibly
         * destined for an ITSP.
         */
        SipURI newSipUri = sipUri;
        if (domain.equals(SipTester.getSutDomainName()) && targetUser != null) {
            String newDomain = SipTester.getTesterConfig().getSipxProxyDomain();
            newSipUri = SipTester.getAddressFactory().createSipURI(targetUser, newDomain);
        }

        SipURI fromSipUri = (SipURI) sipRequest.getFromHeader().getAddress().getURI();
        String fromUser = fromSipUri.getUser();
        String fromDomain = fromSipUri.getHost();

        FromHeader fromHeader = (FromHeader) sipRequest.getFromHeader().clone();

        SutUA fromUa = SipTester.getSutUA(fromUser);

        if (domain.equals(SipTester.getSutDomainName())
                && SipTester.getTestUser(fromUser) != null) {
            String newDomain = SipTester.getTesterConfig().getSipxProxyDomain();
            String newFromUser = SipTester.getTestUser(fromUser);
            SipURI newFromURI = SipTester.getStackBean().getAddressFactory().createSipURI(
                    newFromUser, newDomain);
            Address newFromAddress = SipTester.getAddressFactory().createAddress(newFromURI);
            newFromAddress.setDisplayName(newFromUser);
            String fromTag = sipRequest.getFromHeader().getTag();
            fromHeader = SipTester.getHeaderFactory().createFromHeader(newFromAddress, fromTag);

        }

        Address toAddress = SipTester.getAddressFactory().createAddress(newSipUri);
        ToHeader toHeader = SipTester.getHeaderFactory().createToHeader(toAddress, null);

        CallIdHeader callIdHeader = sipRequest.getCallIdHeader();

        CSeqHeader cseqHeader = sipRequest.getCSeqHeader();

        String viaHost = SipTester.getTesterConfig().getTesterIpAddress();
        int viaPort = endpoint.getPort();
        String transport = endpoint.getSutUA().getDefaultTransport();

        ViaHeader viaHeader = SipTester.getHeaderFactory().createViaHeader(viaHost, viaPort,
                transport, null);

        LinkedList<ViaHeader> viaList = new LinkedList<ViaHeader>();

        viaList.add(viaHeader);

        ContentTypeHeader contentType = (ContentTypeHeader) sipRequest
                .getHeader(ContentTypeHeader.NAME);

        byte[] content = sipRequest.getRawContent();

        MaxForwardsHeader maxForwards = (MaxForwardsHeader) sipRequest
                .getHeader(MaxForwardsHeader.NAME);
        SipURIExt oldSipUri = (SipURIExt) sipRequest.getRequestURI();
        Iterator<String> params = ((SipURIExt) sipRequest.getRequestURI()).getParameterNames();

        while (params.hasNext()) {
            String name = params.next();
            String value = oldSipUri.getParameter(name);
            newSipUri.setParameter(name, value);
        }

        Request newRequest = SipTester.getMessageFactory().createRequest(newSipUri, method,
                callIdHeader, cseqHeader, fromHeader, toHeader, viaList, maxForwards);

        if (content != null) {
            newRequest.setContent(content, sipRequest.getContentTypeHeader());
        }

        if (sipRequest.getHeader(ContactHeader.NAME) != null) {
            ContactHeader contactHeader = SipUtilities.createContactHeader(endpoint);
            newRequest.setHeader(contactHeader);
        }

        if (sipRequest.getHeader(RouteHeader.NAME) != null) {
            Hop hop = new FindSipServer(logger).findServer(newSipUri);
            RouteHeader newRouteHeader = SipUtilities.createRouteHeader(hop);
            sipRequest.setHeader(newRouteHeader);
        }

        SipUtilities.copyHeaders(sipRequest, newRequest);

        return (RequestExt) newRequest;

    }

    public static ResponseExt createResponse(Endpoint endpoint, RequestExt request,
            SipResponse sipResponse) {
        try {
            Response response = sipResponse.getSipResponse();
            int statusCode = response.getStatusCode();
            ResponseExt newResponse = (ResponseExt) SipTester.getMessageFactory().createResponse(
                    statusCode, request);

            String transport = request.getTopmostViaHeader().getTransport();

            ContactHeader contactHeader = SipUtilities.createContactHeader(endpoint
                    .getListeningPoint(transport));
            newResponse.setHeader(contactHeader);
            String toTag = sipResponse.getSipResponse().getToHeader().getTag();

            if (toTag != null) {
                ToHeader newTo = newResponse.getToHeader();
                newTo.setTag(toTag);
            }
            ContentTypeHeader contentTypeHeader = (ContentTypeHeader) response
                    .getHeader(ContentTypeHeader.NAME);
            byte[] content = response.getRawContent();
            if (content != null) {
                newResponse.setContent(content, contentTypeHeader);
            }

            copyHeaders(response, newResponse);

            return newResponse;
        } catch (Exception ex) {
            SipTester.fail("unxpeceted exception", ex);
            return null;
        }
    }

}
