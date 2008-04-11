/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxbridge;

import java.io.IOException;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.SocketException;
import java.nio.ByteBuffer;
import java.nio.channels.ClosedChannelException;
import java.nio.channels.DatagramChannel;
import java.util.Map;
import java.util.Random;
import java.util.TimerTask;
import java.util.Vector;

import javax.sdp.Connection;
import javax.sdp.MediaDescription;
import javax.sdp.Origin;
import javax.sdp.SessionDescription;

import org.apache.commons.beanutils.PropertyUtils;
import org.apache.log4j.Logger;

/**
 * A media endpoint is an ip addres, port pair.
 * 
 * @author M. Ranganathan
 * 
 */
public class SymEndpoint implements SymEndpointInterface {

    private static Logger logger = Logger.getLogger(SymEndpoint.class);

    private String id;

    /*
     * IP address
     */
    private String ipAddress;

    /*
     * My port
     */
    private int port;

    // private InetAddress inetAddress;

    private InetSocketAddress rtpSocketAddress;

    private InetSocketAddress rtcpSocketAddress;

    /*
     * Session description.
     */
    private SessionDescription sessionDescription;

    /*
     * Flag that indicates if this is transmitter or receiver.
     */
    private boolean isTransmitter;

    /*
     * Our datagram channel.
     */
    private DatagramChannel rtpDatagramChannel;

    private Sym sym;

    private int seqno = 1;

    private long ssrc = Math.abs(new Random().nextInt());

    private long creationTime;

    private boolean markerSent = false;

    private int payloadType = 0;

    private boolean earlyMediaStarted = false;

    private boolean onHold = false;

    private EarlyMediaSender earlyMediaSender;

    private DatagramChannel rtcpDatagramChannel;

    private int maxSilence;

    private long lastPacketSentTime;

    private boolean useLastSentForKeepalive;

    private byte[] keepalivePayload = null;

    private ByteBuffer keepAliveBuffer = ByteBuffer.allocate(0);
    
    private boolean remoteAddressAutoDiscoverFlag  = false;

    class EarlyMediaSender extends TimerTask {

        EarlyMediaSender() {

            logger
                    .debug("Starting early media sender TimerTask : remote endpoint = "
                            + SymEndpoint.this.getIpAddress()
                            + "/ "
                            + SymEndpoint.this.port);

        }

        public void run() {
            try {
                if (rtpDatagramChannel == null)
                    return;
                long now = System.currentTimeMillis();
                if (now - lastPacketSentTime < Gateway
                        .getMediaKeepaliveMilisec())
                    return;
                if (useLastSentForKeepalive || keepalivePayload == null) {
                    rtpDatagramChannel.send(keepAliveBuffer, rtpSocketAddress);
                } else if (keepalivePayload != null) {
                    RtpPacket rtpPacket = createRtpPacket();
                    /*
                     * Send a few silence packets to open up the NAT.
                     */
                    rtpPacket.setPayload(keepalivePayload,
                            keepalivePayload.length);

                    if (!markerSent) {
                        rtpPacket.setMarker(1);
                        markerSent = true;
                    } else {
                        rtpPacket.setMarker(0);
                    }
                    keepAliveBuffer = rtpPacket.getData();
                    rtpDatagramChannel.send(keepAliveBuffer, rtpSocketAddress);
                }
            } catch (ClosedChannelException ex) {
                logger
                        .warn(
                                "Exitting early media thread due to closed channel",
                                ex);
                this.cancel();

            } catch (Exception ex) {
                logger
                        .fatal("Unexpected exception in sending early media ",
                                ex);
                this.cancel();
            }

        }

    }

    public Map toMap() {
        try {
            Map retval = PropertyUtils.describe((SymEndpointInterface) this);
            retval.remove("class");
            return retval;
        } catch (Exception ex) {
            logger.error("Error generating map", ex);
            throw new RuntimeException(ex);
        }

    }

    /*
     * Constructor for RTP transmitter.
     */
    public SymEndpoint(boolean isTransmitter) throws IOException {

        this.isTransmitter = isTransmitter;
        this.id = "rtp-endpoint:" + Math.abs(new Random().nextLong());
        if (!isTransmitter) {
            // new Exception().printStackTrace();
            // If this is a reciever, we set our IP address
            // to the local or global address depending upon
            // what the ITSP expects to see.

            this.ipAddress = Gateway.getLocalAddress();

            logger.debug("Creating RtpEndpoint " + ipAddress);

            // This part will probably be replaced by an Iptables splice for now
            // we use NIO. Should scale fairly well but not handle zillions of
            // streams.

            this.rtpDatagramChannel = DatagramChannel.open();

            /* Set up to be non blocking */
            this.rtpDatagramChannel.configureBlocking(false);

            this.rtcpDatagramChannel = DatagramChannel.open();

            this.rtcpDatagramChannel.configureBlocking(false);

            InetAddress inetAddress = InetAddress.getByName(Gateway
                    .getLocalAddress());

            this.rtpSocketAddress = null;
            this.rtcpSocketAddress = null;

            for (this.port = Gateway.getRtpPortRangeLowerBound(); port < Gateway
                    .getRtpPortRangeUpperBound(); port += 2) {
                rtpSocketAddress = new InetSocketAddress(inetAddress, port);
                rtcpSocketAddress = new InetSocketAddress(inetAddress, port + 1);
                try {
                    rtpDatagramChannel.socket().bind(rtpSocketAddress);
                    rtcpDatagramChannel.socket().bind(rtcpSocketAddress);
                    if (logger.isDebugEnabled())
                        logger.debug("allocating port " + rtpSocketAddress);
                    return;
                } catch (SocketException ex) {

                    continue;

                }

            }

            this.creationTime = System.currentTimeMillis();

            if (port == Gateway.getRtpPortRangeUpperBound())
                throw new IOException(
                        "Cannot find a port to bind the rtp endpoint");

        }

    }

    /**
     * Get the unique ID for this endpoint.
     */
    public String getId() {
        return this.id;
    }

    /**
     * Get our RTCP datagram channel.
     * 
     * @return
     */
    public DatagramChannel getRtcpDatagramChannel() {
        return rtcpDatagramChannel;
    }

    /**
     * Get our RTP datagram channel.
     */
    public DatagramChannel getRtpDatagramChannel() {
        return rtpDatagramChannel;
    }

    private RtpPacket createRtpPacket() {
        RtpPacket rtpPacket = new RtpPacket();
        rtpPacket.setSSRC(ssrc);
        rtpPacket.setSequenceNumber(this.seqno++);
        long delta;
        if (this.creationTime == 0) {
            this.creationTime = System.currentTimeMillis();
            delta = 0;
        } else {
            delta = System.currentTimeMillis() - creationTime;
        }

        rtpPacket.setTimeStamp(delta);

        rtpPacket.setPayloadType(this.payloadType); // BUGBUG -- look at the SDP
        // to determine payload
        // type.
        return rtpPacket;

    }

    public RtpPacket createRtpPacket(ByteBuffer readBuffer) {
        RtpPacket rtpPacket = new RtpPacket(readBuffer);
        rtpPacket.setSSRC(ssrc);
        rtpPacket.setSequenceNumber(this.seqno++);
        long delta;
        if (this.creationTime == 0) {
            this.creationTime = System.currentTimeMillis();
            delta = 0;
        } else {
            delta = System.currentTimeMillis() - creationTime;
        }
        long timestamp = rtpPacket.getTimeStamp();

        rtpPacket.setTimeStamp(delta + timestamp);

        return rtpPacket;
    }

    private void startKeepaliveTimer() {
        if (!this.isTransmitter)
            throw new IllegalStateException(
                    "Operation only permitted on transmitters");
        logger.debug("startEarlyMediaThread " + this.toString());
        if (earlyMediaStarted)
            return;
        this.earlyMediaStarted = true;
        this.earlyMediaSender = new EarlyMediaSender();
        Gateway.timer.schedule(this.earlyMediaSender, this.maxSilence,
                this.maxSilence);

    }

    /**
     * Set our session description parameter. Note that for a reciever, we fix
     * up the session description to be the IP address and port that we are
     * listening at.Make sure you call this method only with a cloned session
     * description because the fields of the sessiondescription are rewritten.
     * 
     * @param sessionDescription
     */
    public void setSessionDescription(SessionDescription sessionDescription) {
        if (sessionDescription == null)
            throw new NullPointerException("Null session description! ");
        logger.debug("sessionDescription = " + sessionDescription);

        try {
            this.sessionDescription = sessionDescription;

            if (isTransmitter) {

                if (sessionDescription.getConnection() != null)
                    this.ipAddress = sessionDescription.getConnection()
                            .getAddress();

                // TODO -- we need to sort through different media types here.
                MediaDescription mediaDescription = (MediaDescription) sessionDescription
                        .getMediaDescriptions(true).get(0);

                if (mediaDescription.getConnection() != null) {

                    ipAddress = mediaDescription.getConnection().getAddress();

                }

                this.port = mediaDescription.getMedia().getMediaPort();

                if (logger.isDebugEnabled()) {
                    logger.debug("isTransmitter = true : Setting ipAddress : "
                            + ipAddress);
                    logger.debug("isTransmitter = true : Setting port " + port);
                }

                InetAddress inetAddress = InetAddress.getByName(ipAddress);
                this.rtpSocketAddress = new InetSocketAddress(inetAddress,
                        this.port);

                String attribute = mediaDescription.getAttribute("rtpmap");
                String[] attributes = attribute.split(" ");
                String[] pt = attributes[1].split("/");
                if (logger.isDebugEnabled()) {
                    logger.debug("determining payload type for " + pt[0]);
                }
                this.payloadType = RtpPayloadTypes.getPayloadType(pt[0]);

                /*
                 * We use the same datagram channel for sending and receiving.
                 */
                this.rtpDatagramChannel = this.getRtpSession().getReceiver()
                        .getRtpDatagramChannel();
                if (this.rtpDatagramChannel == null) {
                    logger.error("Setting datagram channel to NULL! ");
                }

                this.rtcpDatagramChannel = this.getRtpSession().getReceiver().rtcpDatagramChannel;

                this.onHold = false;

                assert this.rtpDatagramChannel != null;

            } else {
                Connection connection = sessionDescription.getConnection();

                if (connection != null)
                    connection.setAddress(this.ipAddress);

                Origin origin = sessionDescription.getOrigin();
                origin.setAddress(this.ipAddress);
                Vector mds = sessionDescription.getMediaDescriptions(true);
                for (int i = 0; i < mds.size(); i++) {
                    MediaDescription mediaDescription = (MediaDescription) mds
                            .get(i);
                    if (mediaDescription.getConnection() != null) {
                        mediaDescription.getConnection().setAddress(
                                this.ipAddress);
                    }

                    mediaDescription.getMedia().setMediaPort(port);
                }

                if (logger.isDebugEnabled()) {
                    logger.debug("Setting ipAddress : " + ipAddress);
                    logger.debug("Setting port " + port);
                }

            }
        } catch (Exception ex) {
            logger.error("Unepxected parse exception in sdp", ex);
            return;
        }
    }

    public String getIpAddress() {
        return ipAddress;
    }

    public int getPort() {
        return port;
    }

    public SessionDescription getSessionDescription() {
        return sessionDescription;
    }

    /**
     * @param rtpSession
     *            the rtpSession to set
     */
    public void setRtpSession(Sym rtpSession) {
        this.sym = rtpSession;
    }

    /**
     * @return the rtpSession
     */
    public Sym getRtpSession() {
        return sym;
    }

    public void send(ByteBuffer byteBuffer) throws IOException {
        if (!this.isTransmitter)
            new IOException("This is not a transmitter channel");
        if (logger.isDebugEnabled())
            logger.debug("Sending to " + this.rtpSocketAddress);
        this.lastPacketSentTime = System.currentTimeMillis();
        if (this.useLastSentForKeepalive) {
            this.keepAliveBuffer = byteBuffer;
        }
        this.rtpDatagramChannel.send(byteBuffer, this.rtpSocketAddress);

    }

    public void send(RtpPacket rtpPacket) throws IOException {
        if (!this.isTransmitter)
            new IOException("This is not a transmitter channel");
        /*
         * if ( !this.markerSent) rtpPacket.setMarker(1); else {
         * rtpPacket.setMarker(0); // Note that the marker was already set
         * before. this.markerSent = true; }
         */
        ByteBuffer bbuf = rtpPacket.getData();

        this.send(bbuf);
        this.lastPacketSentTime = System.currentTimeMillis();

    }

    /**
     * @return the earlyMediaStarted
     */
    public boolean isKeepaliveStarted() {
        return earlyMediaStarted;
    }

    public void setOnHold(boolean onHold) {
        this.onHold = onHold;

    }

    public boolean isOnHold() {
        return this.onHold;
    }

    public void setIpAddress(String newIpAddress) {
        this.ipAddress = newIpAddress;
    }

    public void setPort(int port) {
        this.port = port;
    }

    public void setMaxSilence(int maxSilence) {
        logger.debug("RtpEndpoint : setMaxSilence " + maxSilence);
        if (this.earlyMediaStarted) {
            logger.warn("early media started !", new Exception());
            return;
        }
        this.maxSilence = maxSilence;
        if (maxSilence != 0)
            this.startKeepaliveTimer();
    }

    public void setUseLastSentForKeepAlive(boolean useLastSentForKeepalive) {
        this.useLastSentForKeepalive = useLastSentForKeepalive;

    }

    public void setKeepalivePayload(byte[] keepAlivePacketData) {
        this.keepalivePayload = keepAlivePacketData;

    }

    public byte[] getKeepalivePayload() {
        return this.keepalivePayload;
    }

    public void stopKeepalive() {
        if (this.earlyMediaSender != null)
            this.earlyMediaSender.cancel();

    }

    public void setRemoteAddressAutoDiscovery(boolean autoDiscoverFlag) {
       this.remoteAddressAutoDiscoverFlag = autoDiscoverFlag;
        
    }

}
