/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */

package org.sipfoundry.sipxbridge;

import javax.sdp.SessionDescription;

import org.apache.log4j.Logger;
import org.sipfoundry.sipxrelay.SymImpl;

/**
 * An RTP session consists of a transmitter and receiver endpoint. 
 * Both endpoints belong to the same sym.
 */
class RtpSession {

    private static Logger logger = Logger.getLogger(RtpSession.class);

    private SymImpl symImpl;

    private RtpReceiverEndpoint rtpReceiverEndpoint;

    private RtpTransmitterEndpoint rtpTransmitterEndpoint;

    public RtpSession(SymImpl symImpl) {
        this.symImpl = symImpl;
        this.rtpReceiverEndpoint = new RtpReceiverEndpoint(symImpl.getReceiver());
    }

    public SymImpl getSym() {
        return this.symImpl;
    }

    public String getId() {
        return symImpl.getId();
    }

    public RtpReceiverEndpoint getReceiver() {
        return this.rtpReceiverEndpoint;
    }

    public RtpTransmitterEndpoint getTransmitter() {
        return rtpTransmitterEndpoint;
    }

    protected void setTransmitter(RtpTransmitterEndpoint endpoint) {
        logger.debug("setTransmitter " + this.symImpl.getId());
        this.rtpTransmitterEndpoint = endpoint;
        symImpl.setTransmitter(endpoint.getSymTransmitter());
    }

    /**
     * Does the inbound session Description represent a HOLD request
     * for the Rtp Session?
     * 
     * @param sessionDescription
     * @return
     */
    protected boolean isHoldRequest(SessionDescription sessionDescription) {
    	
    	if ( logger.isDebugEnabled()) {
    		logger.debug("isHoldRequest " + sessionDescription);
    	}

        int oldPort = this.getTransmitter().getPort();
        String oldIpAddress = this.getTransmitter().getIpAddress();

        int newport = SipUtilities.getSessionDescriptionMediaPort(sessionDescription);
        String newIpAddress = SipUtilities
                .getSessionDescriptionMediaIpAddress(sessionDescription);

        /*
         * Get the a media attribute -- CAUTION - this only takes care of the first media.
         * Question - what to do when only one media stream is put on hold?
         */

        String mediaAttribute = SipUtilities
                .getSessionDescriptionMediaAttributeDuplexity(sessionDescription);

        String sessionAttribute = SipUtilities.getSessionDescriptionAttribute(sessionDescription);

        if (logger.isDebugEnabled()) {
            logger.debug("mediaAttribute = " + mediaAttribute + "sessionAttribute = "
                    + sessionAttribute);
            logger.debug("mediaIpAddress = " + newIpAddress + " oldPort " + oldPort + " newPort = " + newport );
        }
        /*
         * RFC2543 specified that placing a user on hold was accomplished by setting the
         * connection address to 0.0.0.0. This has been deprecated, since it doesn't allow for
         * RTCP to be used with held streams, and breaks with connection oriented media. However,
         * a UA MUST be capable of receiving SDP with a connection address of 0.0.0.0, in which
         * case it means that neither RTP nor RTCP should be sent to the peer. Whenever the phone
         * puts an external call on hold, it sends a re-INVITE to the gateway with "a=sendonly".
         * Normally, the gateway would respond with "a=recvonly".
         */
        String attribute = sessionAttribute != null ? sessionAttribute : mediaAttribute;
        if (newIpAddress.equals("0.0.0.0") && newport == oldPort) {
            return true;
        } else if (newport == oldPort && oldIpAddress.equals(newIpAddress) && attribute != null
                && (attribute.equals("sendonly") || attribute.equals("inactive"))) {
            return true;
        }
        return false;
    }

    public String getReceiverState() {
        return this.symImpl.getRecieverState();
    }

}
