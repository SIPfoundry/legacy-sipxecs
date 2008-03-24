/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxbridge;

import java.io.IOException;
import java.nio.channels.DatagramChannel;
import java.util.Vector;

import javax.sdp.SessionDescription;

import org.apache.log4j.Logger;

/**
 * Representation of a media session. A media sesison is a pair of media
 * endpoints.
 * 
 * @author M. Ranganathan
 * 
 */
public class RtpSession {

    private static Logger logger = Logger.getLogger(RtpSession.class);

    /*
     * My endpoint for media.
     */
    private RtpEndpoint receiver;

    /*
     * The remote endpoint for media.
     */
    private RtpEndpoint transmitter;

    private RtpBridge rtpBridge;

    public RtpSession(RtpBridge bridge) {
        this.rtpBridge = bridge;

    }

    public void setMyEndpoint(RtpEndpoint myEndpoint) {
        this.receiver = myEndpoint;
        myEndpoint.setRtpSession(this);
    }

    public RtpEndpoint getReceiver() {
        return receiver;
    }

    /**
     * Set the remote endpoint and connect the datagram socket to the remote
     * socket address.
     * 
     * @param hisEndpoint
     */
    public void setRemoteEndpoint(RtpEndpoint hisEndpoint) {

        this.transmitter = hisEndpoint;
        hisEndpoint.setRtpSession(this);

    }

    public RtpEndpoint getTransmitter() {
        return transmitter;
    }

    public void close() {
        logger.debug("Closing channel : " + this.receiver);
        try {
            if (this.receiver != null) {
                if (this.receiver.getRtpDatagramChannel() != null)
                    this.receiver.getRtpDatagramChannel().close();
                if (this.receiver.getRtcpDatagramChannel() != null)
                    this.receiver.getRtcpDatagramChannel().close();
            }
            if (this.transmitter != null) {
                if (this.transmitter.getRtpDatagramChannel() != null)
                    this.transmitter.getRtpDatagramChannel().close();
                if (this.transmitter.getRtcpDatagramChannel() != null)
                    this.transmitter.getRtcpDatagramChannel().close();
            }
        } catch (Exception ex) {
            logger.error("Unexpected exception occured", ex);
        }

    }

    public String toString() {
        return "[ myEndpoint " + this.getReceiver().getIpAddress() + ":"
                + this.getReceiver().getPort() + "\n" + "hisEndpoint "
                + this.getTransmitter().getIpAddress() + ":"
                + this.getTransmitter().getPort() + "]+\n";

    }

    /**
     * @return the rtpBridge
     */
    public RtpBridge getRtpBridge() {
        return rtpBridge;
    }

    public SessionDescription reAssignSessionParameters(
            SessionDescription sessionDescription) {

        int oldPort = this.getTransmitter().getPort();
        String oldIpAddress = this.getTransmitter().getIpAddress();

        int newport = SipUtilities
                .getSessionDescriptionMediaPort(sessionDescription);
        String newIpAddress = SipUtilities
                .getSessionDescriptionMediaIpAddress(sessionDescription);

        String attribute = SipUtilities
                .getSessionDescriptionMediaAttribute(sessionDescription);

        if (newIpAddress.equals("0.0.0.0") && newport == oldPort) {
            /*
             * RFC2543 specified that placing a user on hold was accomplished by
             * setting the connection address to 0.0.0.0. This has been
             * deprecated, since it doesn't allow for RTCP to be used with held
             * streams, and breaks with connection oriented media. However, a UA
             * MUST be capable of receiving SDP with a connection address of
             * 0.0.0.0, in which case it means that neither RTP nor RTCP should
             * be sent to the peer.
             */
            if (logger.isDebugEnabled()) {
                logger.debug("setting media on hold " + this.toString());
            }
            this.getTransmitter().setOnHold(true);

            return this.getReceiver().getSessionDescription();
        } else if (newport == oldPort && oldIpAddress.equals(newIpAddress)) {
            if (attribute == null || attribute.equals("sendrecv")) {
                logger.debug("No session parameters changed!");
                SipUtilities.setSessionDescriptionMediaAttribute(this
                        .getReceiver().getSessionDescription(), "sendrecv");
                this.getTransmitter().setOnHold(false);
            } else if (attribute.equals("sendonly")) {
                logger.debug("Setting media on hold.");
                this.getTransmitter().setOnHold(true);
                /*
                 * Whenever the phone puts an external call on hold, it sends a
                 * re-INVITE to the gateway with "a=sendonly". Normally, the
                 * gateway would respond with "a=recvonly". However, if the
                 * gateway desires to generate MOH for the call, it can generate
                 * SDP specifying "a=inactive". To the phone, this makes it
                 * appear that the external end of the call has also put the
                 * call on hold, and it should cause the phone to not
                 * generate/obtain MOH media.
                 */

                if ( Gateway.getMusicOnHoldName() == null ) {
                    SipUtilities.setSessionDescriptionMediaAttribute(this
                        .getReceiver().getSessionDescription(), "recvonly");
                } else {

                /*
                 * For the standard MOH, the URI is defined to be
                 * <sip:~~mh~@[domain]>. There is thought that other URIs in the
                 * ~~mh~ series can be allocated
                 */
                    throw new RuntimeException("Not yet implemented!");
                }

            }
            return this.getReceiver().getSessionDescription();

        } else {
            if (logger.isDebugEnabled()) {
                logger
                        .debug("Changing Session Parameters -- this is not yet supported oldIpAddress = "
                                + oldIpAddress
                                + " oldPort = "
                                + oldPort
                                + " newIp = "
                                + newIpAddress
                                + " newPort = "
                                + newport);
            }

            SessionDescription retval = this.getReceiver()
                    .getSessionDescription();
            this.getTransmitter().setIpAddress(newIpAddress);
            this.getTransmitter().setPort(newport);
            return retval;
        }

    }

}
