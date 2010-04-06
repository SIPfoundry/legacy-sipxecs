/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.fswitchtester;

import java.text.ParseException;
import java.util.LinkedList;
import java.util.List;
import java.util.Random;

import javax.sdp.MediaDescription;
import javax.sdp.SdpException;
import javax.sdp.SdpFactory;
import javax.sdp.SdpParseException;
import javax.sdp.SessionDescription;
import javax.sip.InvalidArgumentException;
import javax.sip.ListeningPoint;
import javax.sip.SipProvider;
import javax.sip.address.Address;
import javax.sip.address.SipURI;
import javax.sip.header.CSeqHeader;
import javax.sip.header.CallIdHeader;
import javax.sip.header.ContactHeader;
import javax.sip.header.FromHeader;
import javax.sip.header.MaxForwardsHeader;
import javax.sip.header.RouteHeader;
import javax.sip.header.ToHeader;
import javax.sip.header.UserAgentHeader;
import javax.sip.header.ViaHeader;
import javax.sip.message.Message;
import javax.sip.message.Request;

import org.apache.log4j.Logger;

public class SipUtilities {
    private static Logger logger = Logger.getLogger(SipUtilities.class);

    private static String sdpBodyFormat = "v=0\r\n" + "o=- %s 1001 IN IP4 %s\r\n"
            + "s=fswitchtester\r\n" + "c=IN IP4 %s\r\n" + "t=0 0\r\n"
            + "m=audio %d RTP/AVP  0 101\r\n" + "a=rtpmap:0 PCMU/8000\r\n"
            + "a=rtpmap:101 telephone-event/8000\r\n" + "a=fmtp:101 0-11,8\r\n";

    private static String formatSdp(int port) {
        String ipAddress = FreeSwitchTester.getTesterHost();
        String sessionId = Long.toString(Math.abs(new Random().nextLong()));
        return String.format(sdpBodyFormat, sessionId, ipAddress, ipAddress, port);
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
    
    public static int getSessionDescriptionMediaPort(SessionDescription sessionDescription) {
        try {
            MediaDescription mediaDescription = (MediaDescription) sessionDescription
                    .getMediaDescriptions(true).get(0);
            return mediaDescription.getMedia().getMediaPort();
        } catch (Exception ex) {
            throw new RuntimeException("Malformatted sdp", ex);
        }

    }

    /**
     * Create a UA header
     */
    public static UserAgentHeader createUserAgentHeader() {
        try {

            UserAgentHeader userAgent = (UserAgentHeader) ProtocolObjects.headerFactory
                    .createHeader(UserAgentHeader.NAME,
                            "sipXecs/3.11.3 sipXecs/fswitchtester (Linux)");
            return userAgent;
        } catch (ParseException ex) {
            throw new RuntimeException("Unexpected exception ", ex);
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
     * Create a contact header for the given provider.
     */
    public static ContactHeader createContactHeader(String user, SipProvider provider,
            String transport) {
        try {
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

    public static Request createInviteRequest(SipProvider sipProvider, String fromUser) {
        try {
            TesterConfig testerConfig = FreeSwitchTester.getTesterConfig();

            String toUser = testerConfig.getConferenceExtension();

            String toDomain = FreeSwitchTester.getSipxProxyDomain();

            SipURI requestUri;
            if (toUser.contains("@")) {
               requestUri = (SipURI)ProtocolObjects.addressFactory.createURI(toUser);
            } else {
               requestUri = ProtocolObjects.addressFactory.createSipURI(toUser, toDomain);
            }

            SipURI fromUri = ProtocolObjects.addressFactory.createSipURI(fromUser, toDomain);

            Address address = ProtocolObjects.addressFactory.createAddress(fromUri);

            String tag = new Long(Math.abs(new java.util.Random().nextLong())).toString();

            FromHeader fromHeader = ProtocolObjects.headerFactory.createFromHeader(address, tag);

            SipURI toUri = requestUri;

            ToHeader toHeader = ProtocolObjects.headerFactory.createToHeader(
                    ProtocolObjects.addressFactory.createAddress(toUri), null);

            CallIdHeader callid = sipProvider.getNewCallId();

            CSeqHeader cseqHeader = ProtocolObjects.headerFactory.createCSeqHeader(1L,
                    Request.INVITE);

            MaxForwardsHeader maxForwards = ProtocolObjects.headerFactory
                    .createMaxForwardsHeader(70);

            ViaHeader viaHeader = createViaHeader(sipProvider, "udp");

            List<ViaHeader> list = new LinkedList<ViaHeader>();
            list.add(viaHeader);

            Request request = ProtocolObjects.messageFactory.createRequest(requestUri,
                    Request.INVITE, callid, cseqHeader, fromHeader, toHeader, list, maxForwards);

            ContactHeader contactHeader = createContactHeader(fromUser, sipProvider, "udp");
            request.addHeader(contactHeader);

            String outboundProxy = testerConfig.getSipxProxyAddress();
            SipURI routeUri = ProtocolObjects.addressFactory.createSipURI(null, outboundProxy);
            routeUri.setLrParam();
            Address routeAddress = ProtocolObjects.addressFactory.createAddress(routeUri);
            RouteHeader routeHeader = ProtocolObjects.headerFactory
                    .createRouteHeader(routeAddress);
//            request.addHeader(routeHeader);
            request.setHeader(createUserAgentHeader());
            int port = FreeSwitchTester.allocateMediaPort();
            request.setContent(formatSdp(port), ProtocolObjects.headerFactory
                    .createContentTypeHeader("application", "sdp"));
            return request;

        } catch (ParseException ex) {
            String s = "Unexpected error creating INVITE -- check proxy configuration ";
            logger.error(s, ex);
            throw new FreeSwitchTesterException(s, ex);

        } catch (InvalidArgumentException e) {
            String s = "Unexpected error creating INVITE";
            logger.fatal(s, e);
            throw new FreeSwitchTesterException(s, e);
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

}
