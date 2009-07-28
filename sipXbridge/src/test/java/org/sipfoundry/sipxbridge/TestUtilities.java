/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxbridge;

import java.text.ParseException;
import java.util.LinkedList;
import java.util.List;

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
import javax.sip.header.ToHeader;
import javax.sip.header.ViaHeader;
import javax.sip.message.Request;

import org.apache.log4j.Logger;

public class TestUtilities {

    private static Logger logger = Logger.getLogger(TestUtilities.class);

    /**
     * Get the Via header to assign for this message processor. The topmost via
     * header of the outoging messages use this.
     *
     * @return the ViaHeader to be used by the messages sent via this message
     *         processor.
     */
    public static ViaHeader createViaHeader(SipProvider sipProvider) {
        try {
            ListeningPoint listeningPoint = sipProvider
                    .getListeningPoint("udp");
            String host = listeningPoint.getIPAddress();
            int port = listeningPoint.getPort();
            return SipFactories.headerFactory.createViaHeader(host, port,
                    listeningPoint.getTransport(), null);

        } catch (Exception ex) {
            logger.fatal("Unexpected exception creating via header", ex);
            throw new RuntimeException("Could not create via header", ex);
        }
    }

    /**
     * Create a contact header for the given provider.
     */
    public static ContactHeader createContactHeader(SipProvider provider) {
        try {

            ListeningPoint lp = provider.getListeningPoint(ListeningPoint.UDP);
            String ipAddress = lp.getIPAddress();
            int port = lp.getPort();
            SipURI sipUri = SipFactories.addressFactory.createSipURI(null,
                    ipAddress);
            sipUri.setPort(port);
            Address address = SipFactories.addressFactory.createAddress(sipUri);
            ContactHeader ch = SipFactories.headerFactory
                    .createContactHeader(address);
            return ch;

        } catch (Exception ex) {
            throw new RuntimeException(
                    "Unexpected error creating contact header", ex);
        }

    }

    /**
     * Create an invite request bound to the bridge
     *
     * @param sipProvider
     * @param toUser
     * @param toDomain
     * @param isphone
     * @return
     * @throws SipXbridgeException
     */
    public static Request createOutboundInviteRequest(SipProvider sipProvider,
            String toUser, String toDomain, String gatewayAddress, int gatewayPort, boolean isphone)
            throws SipXbridgeException {
        try {

            SipURI requestUri = SipFactories.addressFactory.createSipURI(
                    toUser, gatewayAddress);
            requestUri.setPort(gatewayPort);
            if (isphone) {

                requestUri.setUserParam("phone");
            }

            /*
             * This is from the sipx proxy bound outward.
             */
            SipURI fromUri = SipFactories.addressFactory.createSipURI(null,
                    "proxy.sipx.com");

            FromHeader fromHeader = SipFactories.headerFactory
                    .createFromHeader(SipFactories.addressFactory
                            .createAddress(fromUri), new Long(Math
                            .abs(new java.util.Random().nextLong())).toString());

            SipURI toUri = SipFactories.addressFactory.createSipURI(toUser,
                    toDomain);
            if (isphone) {
                toUri.setUserParam("phone");
            }

            ToHeader toHeader = SipFactories.headerFactory.createToHeader(
                    SipFactories.addressFactory.createAddress(toUri), null);

            CallIdHeader callid = sipProvider.getNewCallId();

            CSeqHeader cseqHeader = SipFactories.headerFactory
                    .createCSeqHeader(1L, Request.INVITE);

            MaxForwardsHeader maxForwards = SipFactories.headerFactory
                    .createMaxForwardsHeader(70);

            ViaHeader viaHeader = createViaHeader(sipProvider);

            List<ViaHeader> list = new LinkedList<ViaHeader>();
            list.add(viaHeader);

            Request request = SipFactories.messageFactory.createRequest(
                    requestUri, Request.INVITE, callid, cseqHeader, fromHeader,
                    toHeader, list, maxForwards);

            ContactHeader contactHeader = createContactHeader(sipProvider);

            request.addHeader(contactHeader);

            return request;

        } catch (ParseException ex) {
            String s = "Unexpected error creating INVITE -- check proxy configuration ";
            logger.error(s, ex);
            throw new SipXbridgeException(s, ex);

        } catch (InvalidArgumentException e) {
            String s = "Unexpected error creating INVITE";
            logger.fatal(s, e);
            throw new RuntimeException(s, e);
        }
    }

}
