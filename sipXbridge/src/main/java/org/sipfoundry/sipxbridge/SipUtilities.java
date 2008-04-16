/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxbridge;

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
    private static UserAgentHeader createUserAgentHeader() {
        try {
            LinkedList<String> tokens = new LinkedList<String>();
            tokens.add("iBridge");
            UserAgentHeader userAgent = ProtocolObjects.headerFactory
                    .createUserAgentHeader(tokens);
            return userAgent;
        } catch (ParseException ex) {
            throw new RuntimeException("Unexpected exception ", ex);
        }
    }

    /**
     * Create a Via header for a given provider and transport.
     */
    public static ViaHeader createViaHeader(SipProvider sipProvider,
            String transport) {
        try {
            ListeningPoint listeningPoint = sipProvider
                    .getListeningPoint(transport);
            String host = listeningPoint.getIPAddress();
            int port = listeningPoint.getPort();
            return ProtocolObjects.headerFactory.createViaHeader(host, port,
                    listeningPoint.getTransport(), null);
        } catch (Exception ex) {
            throw new RuntimeException("Unexpected exception ", ex);
        }

    }

    /**
     * Get the Via header to assign for this message processor. The topmost via
     * header of the outoging messages use this.
     * 
     * @return the ViaHeader to be used by the messages sent via this message
     *         processor.
     */
    public static ViaHeader createViaHeader(SipProvider sipProvider,
            ItspAccountInfo itspAccount) {
        try {
            if (!itspAccount.isGlobalAddressingUsed()) {
                ListeningPoint listeningPoint = sipProvider
                        .getListeningPoint(itspAccount.getOutboundTransport());
                String host = listeningPoint.getIPAddress();
                int port = listeningPoint.getPort();
                ViaHeader viaHeader = ProtocolObjects.headerFactory
                        .createViaHeader(host, port, listeningPoint
                                .getTransport(), null);
                if (itspAccount.isRportUsed())
                    viaHeader.setRPort();
                return viaHeader;

            } else {
                // Check -- what other parameters need to be set for NAT
                // traversal here?

                return ProtocolObjects.headerFactory.createViaHeader(Gateway
                        .getGlobalAddress(), sipProvider.getListeningPoint(
                        itspAccount.getOutboundTransport()).getPort(),
                        itspAccount.getOutboundTransport(), null);

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
            if (!itspAccount.isGlobalAddressingUsed()) {

                ListeningPoint lp = provider
                        .getListeningPoint(ListeningPoint.UDP);
                String ipAddress = lp.getIPAddress();
                int port = lp.getPort();
                SipURI sipUri = ProtocolObjects.addressFactory.createSipURI(
                        itspAccount.getUserName(), ipAddress);
                sipUri.setPort(port);
                Address address = ProtocolObjects.addressFactory
                        .createAddress(sipUri);
                ContactHeader ch = ProtocolObjects.headerFactory
                        .createContactHeader(address);
                ch.removeParameter("expires");
                return ch;

            } else {

                ContactHeader contactHeader = ProtocolObjects.headerFactory
                        .createContactHeader();
                SipURI sipUri = ProtocolObjects.addressFactory.createSipURI(
                        itspAccount.getUserName(), Gateway.getGlobalAddress());
                sipUri.setPort(provider.getListeningPoint(
                        itspAccount.getOutboundTransport()).getPort());
                Address address = ProtocolObjects.addressFactory
                        .createAddress(sipUri);
                contactHeader.setAddress(address);
                contactHeader.removeParameter("expires");
                return contactHeader;
            }
        } catch (Exception ex) {
            logger.fatal("Unexpected exception creating contact header");
            throw new RuntimeException(
                    "Unexpected error creating contact header", ex);
        }

    }

    /**
     * Create a contact header for the given provider.
     */
    public static ContactHeader createContactHeader(String user,
            SipProvider provider) {
        try {
            ListeningPoint lp = provider.getListeningPoint(ListeningPoint.UDP);
            String ipAddress = lp.getIPAddress();
            int port = lp.getPort();
            SipURI sipUri = ProtocolObjects.addressFactory.createSipURI(null,
                    ipAddress);
            if (user != null)
                sipUri.setUser(user);
            sipUri.setPort(port);
            Address address = ProtocolObjects.addressFactory
                    .createAddress(sipUri);
            ContactHeader ch = ProtocolObjects.headerFactory
                    .createContactHeader(address);
            return ch;
        } catch (Exception ex) {
            throw new RuntimeException(
                    "Unexpected error creating contact header", ex);
        }
    }

    /**
     * Create a basic registration request.
     */

    private static Request createRegistrationRequestTemplate(
            ItspAccountInfo itspAccount, SipProvider sipProvider)
            throws ParseException, InvalidArgumentException {

        String proxy = itspAccount.getOutboundProxy();

        SipURI requestUri = ProtocolObjects.addressFactory.createSipURI(null,
                proxy);
        if (itspAccount.getOutboundTransport().equalsIgnoreCase("tcp")) {
            requestUri.setTransportParam("tcp");
        }
        SipURI fromUri = ProtocolObjects.addressFactory.createSipURI(
                itspAccount.getUserName(), itspAccount.getSipDomain());

        SipURI toUri = ProtocolObjects.addressFactory.createSipURI(itspAccount
                .getUserName(), itspAccount.getSipDomain());

        Address fromAddress = ProtocolObjects.addressFactory
                .createAddress(fromUri);
        fromAddress.setDisplayName(itspAccount.getDisplayName());
        FromHeader fromHeader = ProtocolObjects.headerFactory.createFromHeader(
                fromAddress, new Long(Math.abs(new java.util.Random()
                        .nextLong())).toString());

        Address toAddress = ProtocolObjects.addressFactory.createAddress(toUri);
        toAddress.setDisplayName(itspAccount.getDisplayName());
        ToHeader toHeader = ProtocolObjects.headerFactory.createToHeader(
                toAddress, null);

        CallIdHeader callid = sipProvider.getNewCallId();

        CSeqHeader cseqHeader = ProtocolObjects.headerFactory.createCSeqHeader(
                1L, Request.REGISTER);

        MaxForwardsHeader maxForwards = ProtocolObjects.headerFactory
                .createMaxForwardsHeader(70);

        ViaHeader viaHeader = null;

        viaHeader = createViaHeader(sipProvider, itspAccount);

        List<ViaHeader> list = new LinkedList<ViaHeader>();
        list.add(viaHeader);

        Request request = ProtocolObjects.messageFactory.createRequest(
                requestUri, Request.REGISTER, callid, cseqHeader, fromHeader,
                toHeader, list, maxForwards);
        request.addHeader(createUserAgentHeader());

        for (String req : new String[] { Request.INVITE, Request.ACK,
                Request.BYE, Request.CANCEL }) {
            Header header = ProtocolObjects.headerFactory
                    .createAllowHeader(req);
            request.addHeader(header);
        }

        return request;
    }

    /**
     * Creates a deregistration request and sends it out to deregister ourselves
     * from the proxy server.
     * 
     * @param sipProvider
     * @param itspAccount
     * @return
     * @throws GatewayConfigurationException
     */
    public static Request createDeregistrationRequest(SipProvider sipProvider,
            ItspAccountInfo itspAccount) throws GatewayConfigurationException {
        try {

            Request request = createRegistrationRequestTemplate(itspAccount,
                    sipProvider);

            ContactHeader contactHeader = createContactHeader(sipProvider,
                    itspAccount);

            request.addHeader(contactHeader);
            ExpiresHeader expiresHeader = ProtocolObjects.headerFactory
                    .createExpiresHeader(0);
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
            SipURI requestUri = ProtocolObjects.addressFactory.createSipURI(
                    null, itspAccount.getSipDomain());
            // SipURI requestUri =
            // ProtocolObjects.addressFactory.createSipURI(null,
            // itspAccount.getOutboundProxy());
            SipURI fromUri = ProtocolObjects.addressFactory.createSipURI(
                    itspAccount.getUserName(), itspAccount.getSipDomain());

            SipURI toUri = ProtocolObjects.addressFactory.createSipURI(
                    itspAccount.getUserName(), itspAccount.getSipDomain());

            Address fromAddress = ProtocolObjects.addressFactory
                    .createAddress(fromUri);
            fromAddress.setDisplayName(itspAccount.getDisplayName());
            FromHeader fromHeader = ProtocolObjects.headerFactory
                    .createFromHeader(fromAddress, new Long(Math
                            .abs(new java.util.Random().nextLong())).toString());

            Address toAddress = ProtocolObjects.addressFactory
                    .createAddress(toUri);
            toAddress.setDisplayName(itspAccount.getDisplayName());
            ToHeader toHeader = ProtocolObjects.headerFactory.createToHeader(
                    toAddress, null);

            CallIdHeader callid = sipProvider.getNewCallId();

            CSeqHeader cseqHeader = ProtocolObjects.headerFactory
                    .createCSeqHeader(1L, Request.OPTIONS);

            MaxForwardsHeader maxForwards = ProtocolObjects.headerFactory
                    .createMaxForwardsHeader(70);

            ViaHeader viaHeader = null;

            viaHeader = createViaHeader(sipProvider, itspAccount);

            List<ViaHeader> list = new LinkedList<ViaHeader>();
            list.add(viaHeader);

            Request request = ProtocolObjects.messageFactory.createRequest(
                    requestUri, Request.OPTIONS, callid, cseqHeader,
                    fromHeader, toHeader, list, maxForwards);
            request.addHeader(createUserAgentHeader());

            return request;
        } catch (Exception ex) {
            throw new GatewayConfigurationException(
                    "Error creating OPTIONS request", ex);
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
            Request request = createRegistrationRequestTemplate(itspAccount,
                    sipProvider);

            ContactHeader contactHeader = createContactHeader(sipProvider,
                    itspAccount);
            contactHeader.removeParameter("expires");

            request.addHeader(contactHeader);
            int registrationTimer = itspAccount.getSipKeepaliveMethod().equals(
                    Request.REGISTER) ? Gateway.getSipKeepaliveSeconds()
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
    public static Request createRegisterQuery(SipProvider sipProvider,
            ItspAccountInfo itspAccount) throws GatewayConfigurationException {
        try {
            Request request = createRegistrationRequestTemplate(itspAccount,
                    sipProvider);

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

    public static Request createInviteRequest(SipURI requestUri,
            SipProvider sipProvider,
            ItspAccountInfo itspAccount, String user, FromHeader fromHeader,
            String toUser, String toDomain, boolean isphone)
            throws GatewayConfigurationException {
        try {

            

            fromHeader.setTag(new Long(Math.abs(new java.util.Random()
                    .nextLong())).toString());

            SipURI toUri = ProtocolObjects.addressFactory.createSipURI(toUser,
                    toDomain);
            if (isphone) {
                toUri.setUserParam("phone");
            }

            ToHeader toHeader = ProtocolObjects.headerFactory.createToHeader(
                    ProtocolObjects.addressFactory.createAddress(toUri), null);

            CallIdHeader callid = sipProvider.getNewCallId();

            CSeqHeader cseqHeader = ProtocolObjects.headerFactory
                    .createCSeqHeader(1L, Request.INVITE);

            MaxForwardsHeader maxForwards = ProtocolObjects.headerFactory
                    .createMaxForwardsHeader(70);

            ViaHeader viaHeader = createViaHeader(sipProvider, itspAccount);

            List<ViaHeader> list = new LinkedList<ViaHeader>();
            list.add(viaHeader);

            Request request = ProtocolObjects.messageFactory.createRequest(
                    requestUri, Request.INVITE, callid, cseqHeader, fromHeader,
                    toHeader, list, maxForwards);
            SipURI routeURI = ProtocolObjects.addressFactory.createSipURI(null,
                    itspAccount.getOutboundProxy());
            routeURI.setLrParam();
            routeURI.setPort(itspAccount.getProxyPort());
            routeURI.setTransportParam(itspAccount.getOutboundTransport());
            Address address = ProtocolObjects.addressFactory
                    .createAddress(routeURI);
            // request.setHeader(routeHeader);

            Gateway.getAuthenticationHelper().setAuthenticationHeaders(request);

            ContactHeader contactHeader = createContactHeader(sipProvider,
                    itspAccount);

            request.addHeader(contactHeader);

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
        SessionDescription sd = SdpFactory.getInstance()
                .createSessionDescription(messageString);
        return sd;

    }

    public static SessionDescription cleanSessionDescription(
            SessionDescription sessionDescription, String codec) {
        try {
            
            Vector mediaDescriptions = sessionDescription
                    .getMediaDescriptions(true);

            for (Iterator it = mediaDescriptions.iterator(); it.hasNext();) {

                MediaDescription mediaDescription = (MediaDescription) it
                        .next();
                String attribute = mediaDescription.getAttribute("rtpmap");
                String[] attributes = attribute.split(" ");
                String[] pt = attributes[1].split("/");
                logger.debug("pt == " + pt[0]);
                if (!pt[0].equals(codec)) {
                    it.remove();
                    logger.debug("stripping");
                }

            }
            
            return sessionDescription;
        } catch (Exception ex) {
            logger.fatal("Unexpected exception!", ex);
            throw new RuntimeException("Unexpected exception cleaning SDP", ex);
        }
    }

    public static String getSessionDescriptionMediaIpAddress(
            SessionDescription sessionDescription) {
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

    public static String getSessionDescriptionAttribute(
            SessionDescription sessionDescription) {
        try {
            return sessionDescription.getAttribute("a");
        } catch (SdpParseException ex) {
            throw new RuntimeException(
                    "Unexpected exeption retrieving a field", ex);
        }
    }

    public static void setSessionDescriptionMediaAttribute(
            SessionDescription sessionDescription, String originalAttribute,
            String attributeValue) {

        try {

            MediaDescription md = (MediaDescription) sessionDescription
                    .getMediaDescriptions(true).get(0);
            for (Object obj : md.getAttributes(false)) {
                Attribute attr = (Attribute) obj;
                if (attr.getName().equals(originalAttribute))
                    attr.setName(attributeValue);
                return;

            }

        } catch (Exception ex) {
            throw new RuntimeException("Malformatted sdp", ex);
        }

    }

    public static int getSessionDescriptionMediaPort(
            SessionDescription sessionDescription) {
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
        String callId = ((CallIdHeader) message.getHeader(CallIdHeader.NAME))
                .getCallId();

        return callId;
    }

    public static Response createResponse(Transaction transaction,
            int statusCode) {
        try {
            Request request = transaction.getRequest();
            Response response = ProtocolObjects.messageFactory.createResponse(
                    statusCode, request);

            return response;
        } catch (ParseException ex) {
            logger.fatal("Unexpected parse exception", ex);
            throw new RuntimeException("Unexpected parse exceptione", ex);
        }
    }

    public static SipProvider getPeerProvider(SipProvider provider) {
        if (provider == Gateway.getLanProvider())
            return Gateway.getWanProvider();
        else
            return Gateway.getLanProvider();
    }

    public static void fixupSdpAddresses(SessionDescription sessionDescription,
            boolean useGlobalAddress) {
        try {
            Connection connection = sessionDescription.getConnection();
            String address = useGlobalAddress ? Gateway.getGlobalAddress()
                    : Gateway.getLocalAddress();

            if (connection != null)
                connection.setAddress(Gateway.getGlobalAddress());

            Origin origin = sessionDescription.getOrigin();
            origin.setAddress(address);
            Vector mds = sessionDescription.getMediaDescriptions(true);
            for (int i = 0; i < mds.size(); i++) {
                MediaDescription mediaDescription = (MediaDescription) mds
                        .get(i);
                if (mediaDescription.getConnection() != null) {
                    mediaDescription.getConnection().setAddress(address);
                }

            }
        } catch (Exception ex) {
            logger.error("Unepxected exception fixing up sdp addresses", ex);
            throw new RuntimeException(
                    "Unepxected exception fixing up sdp addresses", ex);
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

}
