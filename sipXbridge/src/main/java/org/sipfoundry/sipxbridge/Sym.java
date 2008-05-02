/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxbridge;

import java.io.IOException;
import java.io.Serializable;
import java.nio.channels.DatagramChannel;
import java.util.HashMap;
import java.util.Random;
import java.util.UUID;
import java.util.Vector;

import javax.sdp.SdpParseException;
import javax.sdp.SessionDescription;
import javax.sip.Dialog;
import javax.sip.DialogState;
import javax.sip.SipException;
import javax.sip.message.Request;

import org.apache.log4j.Logger;

/**
 * Representation of a media session. A media sesison is a pair of media
 * endpoints.
 * 
 * @author M. Ranganathan
 * 
 */
public class Sym implements SymInterface, Serializable {

    private static final long serialVersionUID = -2762988912663901623L;

    private static Logger logger = Logger.getLogger(Sym.class);

    private String id;

    private long creationTime;

    long lastPacketTime;

    /*
     * The receier endpoint.
     */
    private SymReceiverEndpoint receiver;

    /*
     * The transmitter endpoint.
     */
    private SymTransmitterEndpoint transmitter;

    /*
     * The bridge to which we blong.
     */
    private Bridge bridge;

    public Sym() {
        id = "sym:" + Math.abs(new Random().nextLong());
        this.creationTime = System.currentTimeMillis();

    }

    public Sym(Bridge bridge) {
        this();
        this.bridge = bridge;

    }

    public HashMap<String, Object> toMap() {
        HashMap<String, Object> retval = new HashMap<String, Object>();
        retval.put("id", id);
        if (this.receiver != null)
            retval.put("receiver", receiver.toMap());
        else {
            retval.put("receiver", new HashMap<String, Object>());
        }
        if (this.transmitter != null) {
            retval.put("transmitter", transmitter.toMap());
        } else {
            retval.put("transmitter", new HashMap<String, Object>());
        }
        return retval;

    }

    /**
     * Set the receiver endpoint. Messages are received at this end.
     * 
     * @param receiverEndpoint
     */
    public void setReceiver(SymReceiverEndpoint receiverEndpoint) {
        this.receiver = receiverEndpoint;
        receiverEndpoint.setSym(this);
    }

    /**
     * Get the receiver endpoint.
     * 
     */
    public SymEndpoint getReceiver() {
        return receiver;
    }

    /**
     * Set the remote endpoint and connect the datagram socket to the remote
     * socket address.
     * 
     * @param hisEndpoint
     */
    public void setTransmitter(SymTransmitterEndpoint hisEndpoint) {

        this.transmitter = hisEndpoint;
        hisEndpoint.setSym(this);
        this.transmitter.datagramChannel = this.receiver.datagramChannel;

    }

    /**
     * Get the transmitter endpoint.
     * 
     */
    public SymTransmitterEndpoint getTransmitter() {
        return transmitter;
    }

    public void close() {
        logger.debug("Closing channel : " + this.receiver);
        try {
            if (this.receiver != null) {
                if (this.receiver.getDatagramChannel() != null) {
                    this.receiver.getDatagramChannel().close();
                } else {
                    logger.debug("receiver datagram channel is null");
                }
                int port = this.receiver.getPort();
                PortRange portRange = new PortRange(port, port + 1);
                Gateway.getPortManager().free(portRange);
                this.receiver = null;

            }
            if (this.transmitter != null) {
                if (this.transmitter.getDatagramChannel() != null)
                    this.transmitter.getDatagramChannel().close();
                else {
                   logger.debug("transmitter datagram channel is null"); 
                }
                this.transmitter.stopKeepalive();
                this.transmitter = null;
            }
            // Return resource to the Port range manager.

        } catch (Exception ex) {
            logger.error("Unexpected exception occured", ex);
        }

    }

    public String toString() {
        StringBuffer sbuf = new StringBuffer();
        sbuf.append("RtpSession = [ ");
        sbuf.append("id = " + this.id);
        if (this.receiver != null) {
            sbuf.append(" RECEIVER : " + this.getReceiver().getIpAddress()
                    + ":" + this.getReceiver().getPort());
        } else {
            sbuf.append("NO RECEIVER");
        }

        if (this.transmitter != null) {
            sbuf.append(": TRANSMITTER : "
                    + this.getTransmitter().getIpAddress() + ":"
                    + this.getTransmitter().getPort());
        } else {
            sbuf.append(": NO TRANSMITTER ");
        }
        sbuf.append("]");
        return sbuf.toString();
    }

    /**
     * @return the rtpBridge
     */
    public Bridge getBridge() {
        return bridge;
    }

    public String getId() {
        return this.id;
    }

    /**
     * Reassign the session parameters ( possibly putting the media on hold and
     * playing music ).
     * 
     * @param sessionDescription
     * @param dat --
     *            the dialog application data
     * @return -- the recomputed session description.
     */
    SessionDescription reAssignSessionParameters(Request request, Dialog dialog)
            throws SdpParseException, SipException {
        SessionDescription sessionDescription = SipUtilities
                .getSessionDescription(request);
        int oldPort = this.getTransmitter().getPort();
        String oldIpAddress = this.getTransmitter().getIpAddress();

        int newport = SipUtilities
                .getSessionDescriptionMediaPort(sessionDescription);
        String newIpAddress = SipUtilities
                .getSessionDescriptionMediaIpAddress(sessionDescription);

        /*
         * Get the a media attribute -- CAUTION - this only takes care of the
         * first media. Question - what to do when only one media stream is put
         * on hold?
         */

        String mediaAttribute = SipUtilities
                .getSessionDescriptionMediaAttributeDuplexity(sessionDescription);

        String sessionAttribute = SipUtilities
                .getSessionDescriptionAttribute(sessionDescription);

        if (logger.isDebugEnabled()) {
            logger.debug("mediaAttribute = " + mediaAttribute);
            logger.debug("sessionAttribute = " + sessionAttribute);
        }

        String attribute = sessionAttribute != null ? sessionAttribute
                : mediaAttribute;

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
            if (Gateway.getMusicOnHoldAddress() != null) {

                /*
                 * For the standard MOH, the URI is defined to be
                 * <sip:~~mh~@[domain]>. There is thought that other URIs in the
                 * ~~mh~ series can be allocated
                 */
                Dialog mohDialog;
                try {
                    mohDialog = Gateway.getCallControlManager()
                            .getBackToBackUserAgent(dialog)
                            .sendInviteToMohServer(
                                    (SessionDescription) this.getReceiver()
                                            .getSessionDescription().clone());
                } catch (CloneNotSupportedException e) {
                    throw new RuntimeException("Unexpected exception ", e);
                }
                DialogApplicationData dat = (DialogApplicationData) dialog
                        .getApplicationData();
                dat.musicOnHoldDialog = mohDialog;

            }
            SipUtilities.setDuplexity(this.getReceiver()
                    .getSessionDescription(), "recvonly");
            SipUtilities.incrementSdpVersion(this.getReceiver()
                    .getSessionDescription());
            return this.getReceiver().getSessionDescription();
        } else if (newport == oldPort && oldIpAddress.equals(newIpAddress)) {
            if (attribute == null || attribute.equals("sendrecv")) {
                logger.debug("Remove media on hold!");
                SipUtilities.setDuplexity(this.getReceiver()
                        .getSessionDescription(), "sendrecv");
                SipUtilities.incrementSdpVersion(this.getReceiver()
                        .getSessionDescription());
                this.getTransmitter().setOnHold(false);

                DialogApplicationData dat = (DialogApplicationData) dialog
                        .getApplicationData();
                if (dat.musicOnHoldDialog != null
                        && dat.musicOnHoldDialog.getState() != DialogState.TERMINATED) {
                    BackToBackUserAgent b2bua = dat.backToBackUserAgent;
                    b2bua.sendByeToMohServer(dat.musicOnHoldDialog);
                }
            } else if (attribute != null && attribute.equals("sendonly")) {
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
                if (Gateway.getMusicOnHoldAddress() != null) {
                    Dialog mohDialog;
                    try {
                        mohDialog = Gateway.getCallControlManager()
                                .getBackToBackUserAgent(dialog)
                                .sendInviteToMohServer(
                                        (SessionDescription) this.getReceiver()
                                                .getSessionDescription()
                                                .clone());
                    } catch (CloneNotSupportedException e) {
                        throw new RuntimeException("Unexpected exception ", e);
                    }
                    DialogApplicationData dat = (DialogApplicationData) dialog
                            .getApplicationData();
                    dat.musicOnHoldDialog = mohDialog;
                }

                SipUtilities.setDuplexity(this.getReceiver()
                        .getSessionDescription(), "recvonly");

                SipUtilities.incrementSdpVersion(this.getReceiver()
                        .getSessionDescription());
            } else if (attribute != null && attribute.equals("inactive")) {
                logger
                        .debug("Setting media on hold. -- saw an inactive Session attribute");
                this.getTransmitter().setOnHold(true);

                if (Gateway.getMusicOnHoldAddress() != null) {
                    Dialog mohDialog;
                    try {
                        mohDialog = Gateway.getCallControlManager()
                                .getBackToBackUserAgent(dialog)
                                .sendInviteToMohServer(
                                        (SessionDescription) this.getReceiver()
                                                .getSessionDescription()
                                                .clone());
                    } catch (CloneNotSupportedException e) {
                        throw new RuntimeException("Unexpected exception ", e);
                    }
                    DialogApplicationData dat = (DialogApplicationData) dialog
                            .getApplicationData();
                    dat.musicOnHoldDialog = mohDialog;
                }
                SipUtilities.setDuplexity(this.getReceiver()
                        .getSessionDescription(), "inactive");

                SipUtilities.incrementSdpVersion(this.getReceiver()
                        .getSessionDescription());
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

    /**
     * @param rtpBridge
     *            the rtpBridge to set
     */
    public void setBridge(Bridge bridge) {
        this.bridge = bridge;
    }

    public SymState getState() {
        if (this.receiver == null && this.transmitter == null)
            return SymState.TERMINATED;
        else if (this.getTransmitter() == null)
            return SymState.INIT;
        else if (this.getTransmitter() != null && getTransmitter().isOnHold()) {
            return SymState.PAUSED;
        } else
            return SymState.RUNNING;
    }

    public long getCreationTime() {
        return this.creationTime;
    }

    public long getLastPacketTime() {
        return this.lastPacketTime;
    }

}
