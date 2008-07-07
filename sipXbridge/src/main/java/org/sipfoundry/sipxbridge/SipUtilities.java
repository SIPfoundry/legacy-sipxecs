/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxbridge;

import gov.nist.javax.sdp.MediaDescriptionImpl;

import java.text.ParseException;
import java.util.Collection;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.Vector;

import javax.sdp.Attribute;
import javax.sdp.Connection;
import javax.sdp.Media;
import javax.sdp.MediaDescription;
import javax.sdp.Origin;
import javax.sdp.SdpException;
import javax.sdp.SdpFactory;
import javax.sdp.SdpParseException;
import javax.sdp.SessionDescription;
import javax.sip.ClientTransaction;
import javax.sip.Dialog;
import javax.sip.InvalidArgumentException;
import javax.sip.ListeningPoint;
import javax.sip.SipProvider;
import javax.sip.Transaction;
import javax.sip.address.Address;
import javax.sip.address.SipURI;
import javax.sip.address.TelURL;
import javax.sip.address.URI;
import javax.sip.header.AuthorizationHeader;
import javax.sip.header.CSeqHeader;
import javax.sip.header.CallIdHeader;
import javax.sip.header.ContactHeader;
import javax.sip.header.ExpiresHeader;
import javax.sip.header.FromHeader;
import javax.sip.header.Header;
import javax.sip.header.HeaderFactory;
import javax.sip.header.MaxForwardsHeader;
import javax.sip.header.RouteHeader;
import javax.sip.header.SupportedHeader;
import javax.sip.header.ToHeader;
import javax.sip.header.UserAgentHeader;
import javax.sip.header.ViaHeader;
import javax.sip.message.Message;
import javax.sip.message.MessageFactory;
import javax.sip.message.Request;
import javax.sip.message.Response;

import org.apache.log4j.Logger;

/**
 * 
 * @author mranga
 * 
 */
public class SipUtilities {

    private static Logger logger = Logger.getLogger(SipUtilities.class);

    /**
     * Create a UA header
     */
    public static UserAgentHeader createUserAgentHeader() {
        try {
            LinkedList<String> tokens = new LinkedList<String>();
            UserAgentHeader userAgent = (UserAgentHeader) ProtocolObjects.headerFactory
                    .createHeader(UserAgentHeader.NAME,
                            "sipXecs/3.11.3 sipXecs/sipxbridge (Linux)");
            return userAgent;
        } catch (ParseException ex) {
            throw new RuntimeException("Unexpected exception ", ex);
        }
    }

    /**
     * Create a Via header for a given provider and transport.
     */
    public static ViaHeader createViaHeader(SipProvider sipProvider, String transport) {
        try {
            if (!transport.equalsIgnoreCase("tcp") && !transport.equalsIgnoreCase("udp"))
                throw new IllegalArgumentException("Bad transport");

            ListeningPoint listeningPoint = sipProvider.getListeningPoint(transport);
            String host = listeningPoint.getIPAddress();
            int port = listeningPoint.getPort();
            return ProtocolObjects.headerFactory.createViaHeader(host, port, listeningPoint
                    .getTransport(), null);
        } catch (Exception ex) {
            throw new RuntimeException("Unexpected exception ", ex);
        }

    }

    /**
     * Get the Via header to assign for this message processor. The topmost via header of the
     * outoging messages use this.
     * 
     * @return the ViaHeader to be used by the messages sent via this message processor.
     */
    public static ViaHeader createViaHeader(SipProvider sipProvider, ItspAccountInfo itspAccount) {
        try {
            if (itspAccount != null && !itspAccount.isGlobalAddressingUsed()) {
                ListeningPoint listeningPoint = sipProvider.getListeningPoint(itspAccount
                        .getOutboundTransport());
                String host = listeningPoint.getIPAddress();
                int port = listeningPoint.getPort();
                ViaHeader viaHeader = ProtocolObjects.headerFactory.createViaHeader(host, port,
                        listeningPoint.getTransport(), null);
                if (itspAccount.isRportUsed())
                    viaHeader.setRPort();
                return viaHeader;

            } else {
                // Check -- what other parameters need to be set for NAT
                // traversal here?
                String transport = itspAccount != null ? itspAccount.getOutboundTransport() :
                    Gateway.DEFAULT_ITSP_TRANSPORT;
                return ProtocolObjects.headerFactory.createViaHeader(Gateway.getGlobalAddress(),
                        Gateway.getGlobalPort(), transport, null);

            }

        } catch (Exception ex) {
            logger.fatal("Unexpected exception creating via header", ex);
            throw new RuntimeException("Could not create via header", ex);
        }
    }

    /**
     * Create a contact header for the given provider.
     */
    public static ContactHeader createContactHeader(SipProvider provider,
            ItspAccountInfo itspAccount) {
        try {
            if (itspAccount != null && !itspAccount.isGlobalAddressingUsed()) {

                ListeningPoint lp = provider
                        .getListeningPoint(itspAccount.getOutboundTransport());
                String ipAddress = lp.getIPAddress();
                int port = lp.getPort();
                SipURI sipUri = ProtocolObjects.addressFactory.createSipURI(itspAccount
                        .getUserName(), ipAddress);
                sipUri.setPort(port);
                sipUri.setTransportParam(itspAccount.getOutboundTransport());
                Address address = ProtocolObjects.addressFactory.createAddress(sipUri);
                ContactHeader ch = ProtocolObjects.headerFactory.createContactHeader(address);
                ch.removeParameter("expires");
                return ch;

            } else {
                // Nothing is known about this ITSP. We just use global addressing.
                
                ContactHeader contactHeader = ProtocolObjects.headerFactory.createContactHeader();
                String userName = itspAccount != null ? itspAccount.getUserName() : null;
                SipURI sipUri = ProtocolObjects.addressFactory.createSipURI(userName, Gateway.getGlobalAddress());           
                sipUri.setPort(Gateway.getGlobalPort());
                String transport = itspAccount != null ? itspAccount.getOutboundTransport() :
                    "udp";
                sipUri.setTransportParam(transport);
                Address address = ProtocolObjects.addressFactory.createAddress(sipUri);
                contactHeader.setAddress(address);
                contactHeader.removeParameter("expires");
                return contactHeader;
            }
        } catch (Exception ex) {
            logger.fatal("Unexpected exception creating contact header");
            throw new RuntimeException("Unexpected error creating contact header", ex);
        }

    }

    /**
     * Create a contact header for the given provider.
     */
    public static ContactHeader createContactHeader(String user, SipProvider provider,
            String transport) {
        try {
            if ( transport == null ) {
                transport = "udp";
            }
            ListeningPoint lp = provider.getListeningPoint(transport);
            String ipAddress = lp.getIPAddress();
            int port = lp.getPort();
            SipURI sipUri = ProtocolObjects.addressFactory.createSipURI(null, ipAddress);
            if (user != null)
                sipUri.setUser(user);
            sipUri.setPort(port);
            sipUri.setTransportParam(transport);
            Address address = ProtocolObjects.addressFactory.createAddress(sipUri);
            ContactHeader ch = ProtocolObjects.headerFactory.createContactHeader(address);
            return ch;
        } catch (Exception ex) {
            throw new RuntimeException("Unexpected error creating contact header", ex);
        }
    }

    /**
     * Create a basic registration request.
     */

    private static Request createRegistrationRequestTemplate(ItspAccountInfo itspAccount,
            SipProvider sipProvider) throws ParseException, InvalidArgumentException {

        String registrar = itspAccount.getOutboundRegistrar() != null ? itspAccount
                .getOutboundRegistrar() : itspAccount.getOutboundProxy();

        SipURI requestUri = ProtocolObjects.addressFactory.createSipURI(null, registrar);
        int port = itspAccount.getProxyPort();
        if (port != 5060) {
            requestUri.setPort(port);
        }
        if (itspAccount.getOutboundTransport().equalsIgnoreCase("tcp")) {
            requestUri.setTransportParam("tcp");
        }
        String proxyDomain = itspAccount.getProxyDomain();
        SipURI fromUri = ProtocolObjects.addressFactory.createSipURI(itspAccount.getUserName(),
                proxyDomain);

        SipURI toUri = ProtocolObjects.addressFactory.createSipURI(itspAccount.getUserName(),
                proxyDomain);

        Address fromAddress = ProtocolObjects.addressFactory.createAddress(fromUri);
        fromAddress.setDisplayName(itspAccount.getDisplayName());
        FromHeader fromHeader = ProtocolObjects.headerFactory.createFromHeader(fromAddress,
                new Long(Math.abs(new java.util.Random().nextLong())).toString());

        Address toAddress = ProtocolObjects.addressFactory.createAddress(toUri);
        toAddress.setDisplayName(itspAccount.getDisplayName());
        ToHeader toHeader = ProtocolObjects.headerFactory.createToHeader(toAddress, null);

        CallIdHeader callid = sipProvider.getNewCallId();

        CSeqHeader cseqHeader = ProtocolObjects.headerFactory.createCSeqHeader(1L,
                Request.REGISTER);

        MaxForwardsHeader maxForwards = ProtocolObjects.headerFactory.createMaxForwardsHeader(70);

        ViaHeader viaHeader = null;

        viaHeader = createViaHeader(sipProvider, itspAccount);

        List<ViaHeader> list = new LinkedList<ViaHeader>();
        list.add(viaHeader);

        Request request = ProtocolObjects.messageFactory.createRequest(requestUri,
                Request.REGISTER, callid, cseqHeader, fromHeader, toHeader, list, maxForwards);
        request.addHeader(createUserAgentHeader());

        for (String req : new String[] {
            Request.INVITE, Request.ACK, Request.BYE, Request.CANCEL
        }) {
            Header header = ProtocolObjects.headerFactory.createAllowHeader(req);
            request.addHeader(header);
        }

        if (itspAccount.getOutboundRegistrarRoute() != null) {
            String outboundRegistrarRoute = itspAccount.getOutboundProxy();
            SipURI routeUri = ProtocolObjects.addressFactory.createSipURI(null,
                    outboundRegistrarRoute);
            routeUri.setLrParam();
            Address routeAddress = ProtocolObjects.addressFactory.createAddress(routeUri);
            RouteHeader routeHeader = ProtocolObjects.headerFactory
                    .createRouteHeader(routeAddress);
            request.addHeader(routeHeader);
        }

        return request;
    }

    /**
     * Creates a deregistration request and sends it out to deregister ourselves from the proxy
     * server.
     * 
     * @param sipProvider
     * @param itspAccount
     * @return
     * @throws GatewayConfigurationException
     */
    public static Request createDeregistrationRequest(SipProvider sipProvider,
            ItspAccountInfo itspAccount) throws GatewayConfigurationException {
        try {

            Request request = createRegistrationRequestTemplate(itspAccount, sipProvider);

            ContactHeader contactHeader = createContactHeader(sipProvider, itspAccount);

            request.addHeader(contactHeader);
            ExpiresHeader expiresHeader = ProtocolObjects.headerFactory.createExpiresHeader(0);
            request.addHeader(expiresHeader);
            return request;
        } catch (ParseException ex) {
            String s = "Unexpected error creating register -- check proxy configuration ";
            logger.error(s, ex);
            throw new GatewayConfigurationException(s, ex);

        } catch (InvalidArgumentException ex) {
            logger.error("An unexpected exception occured", ex);
            throw new RuntimeException("Internal error", ex);
        }

    }

    /**
     * Create an OPTIONS Request
     */

    public static Request createOptionsRequest(SipProvider sipProvider,
            ItspAccountInfo itspAccount) throws GatewayConfigurationException {
        try {
            SipURI requestUri = ProtocolObjects.addressFactory.createSipURI(null, itspAccount
                    .getSipDomain());

            SipURI fromUri = ProtocolObjects.addressFactory.createSipURI(itspAccount
                    .getUserName(), itspAccount.getSipDomain());

            SipURI toUri = ProtocolObjects.addressFactory.createSipURI(itspAccount.getUserName(),
                    itspAccount.getSipDomain());

            Address fromAddress = ProtocolObjects.addressFactory.createAddress(fromUri);
            fromAddress.setDisplayName(itspAccount.getDisplayName());
            FromHeader fromHeader = ProtocolObjects.headerFactory.createFromHeader(fromAddress,
                    new Long(Math.abs(new java.util.Random().nextLong())).toString());

            Address toAddress = ProtocolObjects.addressFactory.createAddress(toUri);
            toAddress.setDisplayName(itspAccount.getDisplayName());
            ToHeader toHeader = ProtocolObjects.headerFactory.createToHeader(toAddress, null);

            CallIdHeader callid = sipProvider.getNewCallId();

            CSeqHeader cseqHeader = ProtocolObjects.headerFactory.createCSeqHeader(1L,
                    Request.OPTIONS);

            MaxForwardsHeader maxForwards = ProtocolObjects.headerFactory
                    .createMaxForwardsHeader(70);

            ViaHeader viaHeader = null;

            viaHeader = createViaHeader(sipProvider, itspAccount);

            List<ViaHeader> list = new LinkedList<ViaHeader>();
            list.add(viaHeader);

            Request request = ProtocolObjects.messageFactory.createRequest(requestUri,
                    Request.OPTIONS, callid, cseqHeader, fromHeader, toHeader, list, maxForwards);
            request.addHeader(createUserAgentHeader());

            return request;
        } catch (Exception ex) {
            throw new GatewayConfigurationException("Error creating OPTIONS request", ex);
        }
    }

    /**
     * Create a Registration request for the given ITSP account.
     * 
     * @param sipProvider
     * @param itspAccount
     * @return
     * @throws GatewayConfigurationException
     */

    public static Request createRegistrationRequest(SipProvider sipProvider,
            ItspAccountInfo itspAccount) throws GatewayConfigurationException {

        try {
            Request request = createRegistrationRequestTemplate(itspAccount, sipProvider);

            ContactHeader contactHeader = createContactHeader(sipProvider, itspAccount);
            contactHeader.removeParameter("expires");

            request.addHeader(contactHeader);
            int registrationTimer = itspAccount.getSipKeepaliveMethod().equals(Request.REGISTER) ? Gateway
                    .getSipKeepaliveSeconds()
                    : itspAccount.getRegistrationInterval();
            ExpiresHeader expiresHeader = ProtocolObjects.headerFactory
                    .createExpiresHeader(registrationTimer);
            request.addHeader(expiresHeader);
            return request;
        } catch (ParseException ex) {
            String s = "Unexpected error creating register -- check proxy configuration ";
            logger.error(s, ex);
            throw new GatewayConfigurationException(s, ex);

        } catch (InvalidArgumentException ex) {
            logger.error("An unexpected exception occured", ex);
            throw new RuntimeException("Internal error", ex);
        }

    }

    /**
     * Create a registration query.
     * 
     * @param sipProvider
     * @param itspAccount
     * @return
     * @throws GatewayConfigurationException
     */
    public static Request createRegisterQuery(SipProvider sipProvider, ItspAccountInfo itspAccount)
            throws GatewayConfigurationException {
        try {
            Request request = createRegistrationRequestTemplate(itspAccount, sipProvider);

            return request;
        } catch (ParseException ex) {
            String s = "Unexpected error creating register -- check proxy configuration ";
            logger.error(s, ex);
            throw new GatewayConfigurationException(s, ex);

        } catch (InvalidArgumentException ex) {
            logger.error("An unexpected exception occured", ex);
            throw new RuntimeException("Internal error", ex);
        }

    }

    public static Request createInviteRequest(SipURI requestUri, SipProvider sipProvider,
            ItspAccountInfo itspAccount, FromHeader from, String toUser, String toDomain,
            boolean isphone) throws GatewayConfigurationException {
        try {
            FromHeader fromHeader = null;

            /*
             * If the account requires a register on init, then use the From header that was
             * Registered.
             */
            if (itspAccount.isRegisterOnInitialization()
                    && itspAccount.isUseRegistrationForCallerId()) {
                String domain = itspAccount.getOutboundRegistrar() != null ? itspAccount
                        .getOutboundRegistrar() : itspAccount.getOutboundProxy();
                SipURI fromUri = ProtocolObjects.addressFactory.createSipURI(itspAccount
                        .getUserName(), domain);
                fromUri.removeParameter("user");
                fromHeader = ProtocolObjects.headerFactory.createFromHeader(
                        ProtocolObjects.addressFactory.createAddress(fromUri), new Long(Math
                                .abs(new java.util.Random().nextLong())).toString());

                toDomain = domain;
                toUser = requestUri.getUser();

            } else {
                fromHeader = from;
            }

            if (!isphone) {
                requestUri.removeParameter("user");
            } else {
                requestUri.setUserParam("phone");
            }

            fromHeader.setTag(new Long(Math.abs(new java.util.Random().nextLong())).toString());

            SipURI toUri = ProtocolObjects.addressFactory.createSipURI(toUser, toDomain);

            if (isphone) {
                toUri.setUserParam("phone");
            } else {
                toUri.removeParameter("user");
            }

            ToHeader toHeader = ProtocolObjects.headerFactory.createToHeader(
                    ProtocolObjects.addressFactory.createAddress(toUri), null);

            CallIdHeader callid = sipProvider.getNewCallId();

            CSeqHeader cseqHeader = ProtocolObjects.headerFactory.createCSeqHeader(1L,
                    Request.INVITE);

            MaxForwardsHeader maxForwards = ProtocolObjects.headerFactory
                    .createMaxForwardsHeader(70);

            ViaHeader viaHeader = createViaHeader(sipProvider, itspAccount);

            List<ViaHeader> list = new LinkedList<ViaHeader>();
            list.add(viaHeader);

            Request request = ProtocolObjects.messageFactory.createRequest(requestUri,
                    Request.INVITE, callid, cseqHeader, fromHeader, toHeader, list, maxForwards);

            Gateway.getAuthenticationHelper().setAuthenticationHeaders(request);

            ContactHeader contactHeader = createContactHeader(sipProvider, itspAccount);
            request.addHeader(contactHeader);

            String outboundProxy = itspAccount.getOutboundProxy();
            SipURI routeUri = ProtocolObjects.addressFactory.createSipURI(null, outboundProxy);
            routeUri.setLrParam();
            Address routeAddress = ProtocolObjects.addressFactory.createAddress(routeUri);
            RouteHeader routeHeader = ProtocolObjects.headerFactory
                    .createRouteHeader(routeAddress);
            request.addHeader(routeHeader);
            request.setHeader(createUserAgentHeader());
            return request;

        } catch (ParseException ex) {
            String s = "Unexpected error creating INVITE -- check proxy configuration ";
            logger.error(s, ex);
            throw new GatewayConfigurationException(s, ex);

        } catch (InvalidArgumentException e) {
            String s = "Unexpected error creating INVITE";
            logger.fatal(s, e);
            throw new RuntimeException(s, e);
        }
    }

    public static SessionDescription getSessionDescription(Message message)
            throws SdpParseException {
        if (message.getRawContent() == null)
            throw new SdpParseException(0, 0, "Missing sdp body");
        String messageString = new String(message.getRawContent());
        SessionDescription sd = SdpFactory.getInstance().createSessionDescription(messageString);
        return sd;

    }

    /**
     * Cleans the Session description to include only the specified codec.This processing can be
     * applied on the outbound INVITE to make sure that call transfers will work in the absence of
     * re-invites. It removes all the SRTP related fields as well.
     * 
     * @param sessionDescription
     * @param codec
     * @return
     */
    public static SessionDescription cleanSessionDescription(
            SessionDescription sessionDescription, String codec) {
        try {
            if (codec == null)
                return sessionDescription;
            /*
             * No codec specified -- return the incoming session description.
             */

            boolean found = false;

            Vector mediaDescriptions = sessionDescription.getMediaDescriptions(true);

            int keeper = codec == null ? -1 : RtpPayloadTypes.getPayloadType(codec);

            for (Iterator it = mediaDescriptions.iterator(); it.hasNext();) {

                MediaDescription mediaDescription = (MediaDescription) it.next();
                Vector formats = mediaDescription.getMedia().getMediaFormats(true);
                if (keeper != -1) {
                    for (Iterator it1 = formats.iterator(); it1.hasNext();) {
                        Object format = it1.next();
                        int fmt = new Integer(format.toString());
                        if (fmt != keeper && RtpPayloadTypes.isPayload(fmt))
                            it1.remove();
                        else if (fmt == keeper)
                            found = true;
                    }
                }
                Vector attributes = mediaDescription.getAttributes(true);

                for (Iterator it1 = attributes.iterator(); it1.hasNext();) {
                    Attribute attr = (Attribute) it1.next();
                    if (logger.isDebugEnabled()) {
                        logger.debug("attrName = " + attr.getName());
                    }
                    if (attr.getName().equalsIgnoreCase("rtpmap") && codec != null) {
                        String attribute = attr.getValue();
                        String[] attrs = attribute.split(" ");
                        String[] pt = attrs[1].split("/");
                        if (logger.isDebugEnabled())
                            logger.debug("pt == " + pt[0]);
                        if (RtpPayloadTypes.isPayload(pt[0]) && !pt[0].equalsIgnoreCase(codec)) {
                            it1.remove();
                        }
                    } else if (attr.getName().equalsIgnoreCase("crypto")) {
                        it1.remove();
                    } else if (attr.getName().equalsIgnoreCase("encryption")) {
                        it1.remove();
                    }
                }

            }

            return sessionDescription;

        } catch (Exception ex) {
            logger.fatal("Unexpected exception!", ex);
            throw new RuntimeException("Unexpected exception cleaning SDP", ex);
        }
    }

    public static String getCodecName(Response response) {
        try {
            if (response.getContentLength().getContentLength() == 0)
                return null;

            SessionDescription sd = getSessionDescription(response);
            Vector mediaDescriptions = sd.getMediaDescriptions(true);
            MediaDescription mediaDescription = (MediaDescription) sd.getMediaDescriptions(true)
                    .get(0);
            String attribute = mediaDescription.getAttribute("rtpmap");
            String[] attributes = attribute.split(" ");
            String[] pt = attributes[1].split("/");
            return pt[0];

        } catch (Exception ex) {
            logger.fatal("Unexpected exception!", ex);
            throw new RuntimeException("Unexpected exception cleaning SDP", ex);
        }

    }

    public static String getSessionDescriptionMediaIpAddress(SessionDescription sessionDescription) {
        try {
            String ipAddress = null;
            if (sessionDescription.getConnection() != null)
                ipAddress = sessionDescription.getConnection().getAddress();

            MediaDescription mediaDescription = (MediaDescription) sessionDescription
                    .getMediaDescriptions(true).get(0);

            if (mediaDescription.getConnection() != null) {

                ipAddress = mediaDescription.getConnection().getAddress();

            }
            return ipAddress;
        } catch (SdpParseException ex) {
            throw new RuntimeException("Unexpected parse exception ", ex);
        } catch (SdpException ex) {
            throw new RuntimeException("Unexpected Sdpexception exception ", ex);
        }
    }

    public static String getSessionDescriptionMediaAttributeDuplexity(
            SessionDescription sessionDescription) {
        String retval = null;
        try {

            MediaDescription md = (MediaDescription) sessionDescription
                    .getMediaDescriptions(true).get(0);
            for (Object obj : md.getAttributes(false)) {
                Attribute attr = (Attribute) obj;
                if (attr.getName().equals("sendrecv"))
                    return "sendrecv";
                else if (attr.getName().equals("sendonly"))
                    return "sendonly";
                else if (attr.getName().equals("recvonly"))
                    return "recvonly";
                else if (attr.getName().equals("inactive"))
                    return "inactive";

            }
            return null;
        } catch (Exception ex) {
            throw new RuntimeException("Malformatted sdp", ex);
        }

    }

    public static String getSessionDescriptionAttribute(SessionDescription sessionDescription) {
        try {
            return sessionDescription.getAttribute("a");
        } catch (SdpParseException ex) {
            throw new RuntimeException("Unexpected exeption retrieving a field", ex);
        }
    }

    public static void setDuplexity(SessionDescription sessionDescription, String attributeValue) {

        try {

            MediaDescriptionImpl md = (MediaDescriptionImpl) sessionDescription
                    .getMediaDescriptions(true).get(0);
            md.setDuplexity(attributeValue);

        } catch (Exception ex) {
            logger.error("Error while processing the following SDP : " + sessionDescription);
            logger.error("attributeValue = " + attributeValue);
            throw new RuntimeException("Malformatted sdp", ex);
        }

    }

    public static int getSessionDescriptionMediaPort(SessionDescription sessionDescription) {
        try {
            MediaDescription mediaDescription = (MediaDescription) sessionDescription
                    .getMediaDescriptions(true).get(0);
            return mediaDescription.getMedia().getMediaPort();
        } catch (Exception ex) {
            throw new RuntimeException("Malformatted sdp", ex);
        }

    }

    public static long getSeqNumber(Message message) {
        return ((CSeqHeader) message.getHeader(CSeqHeader.NAME)).getSeqNumber();
    }

    public static String getCallId(Message message) {
        String callId = ((CallIdHeader) message.getHeader(CallIdHeader.NAME)).getCallId();

        return callId;
    }

    public static Response createResponse(Transaction transaction, int statusCode) {
        try {
            Request request = transaction.getRequest();
            Response response = ProtocolObjects.messageFactory
                    .createResponse(statusCode, request);
            SupportedHeader sh = ProtocolObjects.headerFactory.createSupportedHeader("replaces");
            response.addHeader(sh);

            return response;
        } catch (ParseException ex) {
            logger.fatal("Unexpected parse exception", ex);
            throw new RuntimeException("Unexpected parse exceptione", ex);
        }
    }

    public static SipProvider getPeerProvider(SipProvider provider, String transport) {
        if ( transport == null ) {
            transport = Gateway.DEFAULT_ITSP_TRANSPORT;
        }
        if (provider == Gateway.getLanProvider()) {
            return Gateway.getWanProvider(transport);
        } else {
            return Gateway.getLanProvider();
        }
    }

    public static void fixupSdpAddresses(SessionDescription sessionDescription,
            boolean useGlobalAddress) {
        try {
            Connection connection = sessionDescription.getConnection();
            String address = useGlobalAddress ? Gateway.getGlobalAddress() : Gateway
                    .getLocalAddress();

            if (connection != null)
                connection.setAddress(address);

            Origin origin = sessionDescription.getOrigin();
            origin.setAddress(address);
            Vector mds = sessionDescription.getMediaDescriptions(true);
            for (int i = 0; i < mds.size(); i++) {
                MediaDescription mediaDescription = (MediaDescription) mds.get(i);
                if (mediaDescription.getConnection() != null) {
                    mediaDescription.getConnection().setAddress(address);
                }

            }
        } catch (Exception ex) {
            logger.error("Unepxected exception fixing up sdp addresses", ex);
            throw new RuntimeException("Unepxected exception fixing up sdp addresses", ex);
        }

    }

    public static void setSessionDescriptionAttribute(String attribute,
            SessionDescription sessionDescription) {
        try {
            sessionDescription.setAttribute("a", attribute);
        } catch (SdpException ex) {
            logger.error("Unexpected exception ", ex);
            throw new RuntimeException("Unexpected exception", ex);
        }

    }

    
    /**
     * Increment the session version.
     * 
     * @param sessionDescription
     */
    public static void incrementSessionVersion(SessionDescription sessionDescription) {
        try {
            long version = sessionDescription.getOrigin().getSessionVersion();
            sessionDescription.getOrigin().setSessionVersion(++version);
        } catch (SdpException ex) {
            logger.error("Unexpected exception ", ex);
            throw new RuntimeException("Unexepcted exception", ex);
        }

    }

    /**
     * Fix up request to use global addressing.
     * 
     * @param request
     */
    public static void setGlobalAddresses(Request request) {
        try {
          SipURI sipUri = 
              ProtocolObjects.addressFactory.createSipURI(null,  Gateway.getGlobalAddress());
          
          ContactHeader contactHeader = (ContactHeader) request.getHeader(ContactHeader.NAME);
          contactHeader.getAddress().setURI(sipUri);
          ViaHeader viaHeader = (ViaHeader) request.getHeader(ViaHeader.NAME);
          viaHeader.setHost(Gateway.getGlobalAddress());
          viaHeader.setPort(Gateway.getGlobalPort());
        } catch ( Exception ex) {
            logger.error("Unexpected exception ", ex);
            throw new RuntimeException("Unexepcted exception", ex);
        }
         
        
    }

}
