package org.sipfoundry.siptester;

import gov.nist.javax.sip.ListeningPointExt;
import gov.nist.javax.sip.SipStackExt;
import gov.nist.javax.sip.address.SipURIExt;
import gov.nist.javax.sip.header.HeaderFactoryExt;
import gov.nist.javax.sip.header.HeaderFactoryImpl;
import gov.nist.javax.sip.header.extensions.ReferredByHeader;
import gov.nist.javax.sip.header.extensions.ReplacesHeader;
import gov.nist.javax.sip.message.MessageExt;
import gov.nist.javax.sip.message.MessageFactoryExt;
import gov.nist.javax.sip.message.RequestExt;
import gov.nist.javax.sip.message.ResponseExt;

import java.net.InetAddress;
import java.net.URLDecoder;
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
import javax.sip.header.ExtensionHeader;
import javax.sip.header.FromHeader;
import javax.sip.header.Header;
import javax.sip.header.HeaderFactory;
import javax.sip.header.MaxForwardsHeader;
import javax.sip.header.ReasonHeader;
import javax.sip.header.RecordRouteHeader;
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

    static Request createRegistrationRequestTemplate(String userName, EmulatedEndpoint endpoint,
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
            EmulatedEndpoint endpoint, String callId, long cseq) throws SipException,
            SipTesterException {

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
            logger.fatal(s, ex);
            throw new SipTesterException(s, ex);

        }

    }

    private static ContactHeader createContactHeader(EmulatedEndpoint endpoint) {
        try {
            String ipAddress = endpoint.getIpAddress();
            int port = endpoint.getPort();
            SipURI sipUri = SipTester.getAddressFactory().createSipURI(null, ipAddress);
            sipUri.setPort(port);
            Address address = SipTester.getAddressFactory().createAddress(sipUri);
            return SipTester.getHeaderFactory().createContactHeader(address);
        } catch (Exception ex) {
            logger.fatal("Unexpected exception", ex);
            throw new SipTesterException(ex);
        }
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
            SipURI newSipUri = SipUtilities.mapUri(sipUri);
            return SipTester.getAddressFactory().createAddress(newSipUri);
        } catch (Exception ex) {
            SipTester.fail("Unexpected exception", ex);
            throw new SipTesterException(ex);
        }
    }

    /**
     * This routine copies headers from inbound to outbound responses.
     * 
     * @param message
     * @param newMessage
     */
    public static void copyHeaders(Message message, SipMessage triggerMessage, Message newMessage) {
        try {

            if (triggerMessage instanceof SipRequest) {
                SipRequest sipRequest = (SipRequest) triggerMessage;
                Request request = sipRequest.getRequestEvent().getRequest();
                ReferToHeader referTo = (ReferToHeader) request.getHeader(ReferToHeader.NAME);
                if (referTo != null) {
                    SipURIExt uri = (SipURIExt) referTo.getAddress().getURI();
                    Iterator<String> headerNames = uri.getHeaderNames();
                    while (headerNames.hasNext()) {
                        String headerName = headerNames.next();
                        String headerValue = URLDecoder
                                .decode(uri.getHeader(headerName), "UTF-8");
                        newMessage.setHeader(SipTester.getHeaderFactory().createHeader(
                                headerName, headerValue));
                    }
                }
            }
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
                            newHeader = SipTester.getHeaderFactory().createReferToHeader(
                                    newAddress);
                        } else if (newHeader.getName().equals(ReferredByHeader.NAME)) {
                            ReferredByHeader referToHeader = (ReferredByHeader) newHeader;
                            Address address = referToHeader.getAddress();
                            Address newAddress = remapAddress(address);
                            newHeader = ((HeaderFactoryExt) SipTester.getHeaderFactory())
                                    .createReferredByHeader(newAddress);

                        } else if (newHeader.getName().equals(RouteHeader.NAME)) {
                            RouteHeader routeHeader = (RouteHeader) newHeader;
                            Address address = routeHeader.getAddress();
                            Address newAddress = remapAddress(address);
                            newHeader = ((HeaderFactoryExt) SipTester.getHeaderFactory())
                                    .createRouteHeader(newAddress);

                        }
                        newMessage.addHeader(newHeader);
                    }
                }
            }
            String oldBranch = ((MessageExt) message).getTopmostViaHeader().getBranch();
            Header newHeader = SipTester.getHeaderFactory().createHeader(
                    "x-sipx-original-branch", oldBranch);
            newMessage.setHeader(newHeader);
            newMessage.removeHeader(RecordRouteHeader.NAME);

            if (message.getContent() != null) {
                ContentTypeHeader cth = ((MessageExt) message).getContentTypeHeader();
                byte[] contents = message.getRawContent();
                newMessage.setContent(contents, cth);
            }
        } catch (Exception ex) {
            SipTester.fail("unexepcted exception", ex);
        }

    }

    public static SipURI mapUri(SipURI uri) {
        try {
            /*
             * Find the UserAgent that corresponds to the destination of the INVITE.
             */
            logger.debug("mapUri " + uri);
            String toDomain = uri.getHost();
            int toPort = uri.getPort();
            String targetUser = uri.getUser();

            if (targetUser != null) {
                ValidUsersXML validUsers = SipTester.getTraceValidUsers();
                if (toDomain.equals(SipTester.getTraceDomainName())) {
                    for (User user : ValidUsersXML.GetUsers()) {
                        if ((targetUser.equals(user.getUserName()) || user.getAliases().contains(
                                targetUser))) {
                            targetUser = SipTester.getMappedUser(user.getUserName());
                            break;
                        }
                    }
                }
            }

            String newToDomain = toPort == -1 ? SipTester.getMappedAddress(toDomain) : SipTester
                    .getMappedAddress(toDomain + ":" + toPort);
            String[] hostPort = newToDomain.split(":");
            SipURIExt retval = (SipURIExt) SipTester.getAddressFactory().createSipURI(targetUser,
                    hostPort[0]);
            if (hostPort.length > 1) {
                int newPort = Integer.parseInt(hostPort[1]);
                retval.setPort(newPort);
            }
            Iterator<String> names = uri.getParameterNames();
            while (names.hasNext()) {
                String name = names.next();
                String value = uri.getParameter(name);
                if (value != null) {
                    retval.setParameter(name, value);
                }
                if (name.equals("lr")) {
                    retval.setLrParam();
                }

            }
            logger.debug("mappedUri = " + retval);
            return retval;

        } catch (Exception ex) {
            throw new SipTesterException(ex);
        }

    }

    /**
     * Creates an emulated request based upon the request to emulate and the mapping provided for
     * the domains and user names.
     * 
     * @param sipRequest - captured INVITE.
     * 
     * @return emulated INVITE or null
     */
    public static RequestExt createRequest(RequestExt sipRequest, SipMessage triggeringMessage,
            EmulatedEndpoint endpoint) throws Exception {
        SipURI requestUri = (SipURI) sipRequest.getRequestURI();
        SipURI toSipUri = (SipURI) sipRequest.getToHeader().getAddress().getURI();
        SipURI fromSipUri = (SipURI) sipRequest.getFromHeader().getAddress().getURI();
        String method = sipRequest.getMethod();
        SipProvider provider = endpoint.getProvider("udp");
        SipURI newRequestUri = mapUri(requestUri);
        SipURI newFromURI = mapUri(fromSipUri);
        Address newFromAddress = SipTester.getAddressFactory().createAddress(newFromURI);
        String fromTag = sipRequest.getFromHeader().getTag();
        FromHeader fromHeader = SipTester.getHeaderFactory().createFromHeader(newFromAddress,
                fromTag);
        Address toAddress = SipTester.getAddressFactory().createAddress(mapUri(toSipUri));
        ToHeader toHeader = SipTester.getHeaderFactory().createToHeader(toAddress, null);

        // CallIdHeader callIdHeader = sipRequest.getCallIdHeader();
        CallIdHeader callIdHeader = provider.getNewCallId();

        CSeqHeader cseqHeader = sipRequest.getCSeqHeader();

        String viaHost = SipTester.getTesterConfig().getTesterIpAddress();
        int viaPort = endpoint.getPort();
        String transport = endpoint.getSutUA().getDefaultTransport();

        ViaHeader viaHeader = SipTester.getHeaderFactory().createViaHeader(viaHost, viaPort,
                transport, null);

        LinkedList<ViaHeader> viaList = new LinkedList<ViaHeader>();

        viaList.add(viaHeader);

        byte[] content = sipRequest.getRawContent();

        MaxForwardsHeader maxForwards = (MaxForwardsHeader) sipRequest
                .getHeader(MaxForwardsHeader.NAME);

        Request newRequest = SipTester.getMessageFactory().createRequest(newRequestUri, method,
                callIdHeader, cseqHeader, fromHeader, toHeader, viaList, maxForwards);

        if (content != null) {
            newRequest.setContent(content, sipRequest.getContentTypeHeader());
        }

        if (sipRequest.getHeader(ContactHeader.NAME) != null) {
            ContactHeader contactHeader = SipUtilities.createContactHeader(endpoint);
            newRequest.setHeader(contactHeader);
        }

        Iterator<Header> routeIterator = sipRequest.getHeaders(RouteHeader.NAME);

        while (routeIterator.hasNext()) {
            RouteHeader routeHeader = (RouteHeader) routeIterator.next();
            logger.debug("routeHeader = " + routeHeader);
            SipURI routeUri = (SipURI) routeHeader.getAddress().getURI();
            SipURI newSipUri = SipUtilities.mapUri(routeUri);
            Address routeAddress = SipTester.getAddressFactory().createAddress(newSipUri);
            RouteHeader newRouteHeader = SipTester.getHeaderFactory().createRouteHeader(
                    routeAddress);
            logger.debug("newRouteHeader " + newRouteHeader);
            newRequest.addHeader(newRouteHeader);
        }

        SipUtilities.copyHeaders(sipRequest, triggeringMessage, newRequest);

        newRequest.removeHeader(RecordRouteHeader.NAME);
        return (RequestExt) newRequest;

    }

    public static ResponseExt createResponse(EmulatedEndpoint endpoint, RequestExt request,
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

            copyHeaders(response, null, newResponse);

            return newResponse;
        } catch (Exception ex) {
            SipTester.fail("unxpeceted exception", ex);
            return null;
        }
    }

    /**
     * Returns the branch-id parameter from the References header or the bottom most via header if
     * References does not exist.
     */
    public static String getBranchMatchId(Request request) {
        Iterator headers = request.getHeaders(ViaHeader.NAME);
        ExtensionHeader extensionHeader = (ExtensionHeader) request.getHeader("References");
        if (extensionHeader != null) {
            String value = extensionHeader.getValue().trim();
            String[] parts1 = value.split(";rel=");
            String callId1 = parts1[0];
            String[] parts2 = parts1[1].split(";x-sipx-branch=");
            String rel = parts2[0];
            String branchId = null;
            if (parts2.length == 2) {
                branchId = parts2[1].toLowerCase();
            }

            if (branchId != null)
                return branchId;
        }

        String bid = null;
        while (headers.hasNext()) {
            ViaHeader viaHeader = (ViaHeader) headers.next();
            bid = viaHeader.getBranch();

        }

        return bid;

    }

}
