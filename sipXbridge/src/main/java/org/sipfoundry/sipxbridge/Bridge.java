/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxbridge;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.ClosedChannelException;
import java.nio.channels.DatagramChannel;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.util.HashSet;
import java.util.Iterator;
import java.util.Random;
import java.util.Set;

import javax.sdp.SdpFactory;
import javax.sdp.SdpParseException;
import javax.sdp.SessionDescription;
import javax.sip.message.Request;

import org.apache.log4j.Logger;

/**
 * Manages the pairing of Rtp pipes. One side belongs to the Lan the other side
 * terminates the ITSP. There is a data shuffler that reads from one end and
 * writes to the other side of the bridge. This method can potentially benifit
 * from a kernel level copy such as ip-tables.
 * 
 * 
 * TODO -- optimize this to use a thread pool.
 * 
 * 
 * @author M. Ranganathan
 * 
 */
public class Bridge {
    private static Logger logger = Logger.getLogger(Bridge.class);

    private ConcurrentSet<Sym> sessions = new ConcurrentSet<Sym>();

    private boolean started;

    private Thread dataShufflerThread;

    private DataShuffler dataShuffler;

    private boolean earlyMedia = true;

    private BridgeState state = BridgeState.INITIAL;

    SessionDescription sessionDescription;

    boolean initializeSelectors = true;

    private String id;

    private long processingCount = 0;

    private boolean parityFlag;

    class DataShuffler implements Runnable {
        // The buffer into which we'll read data when it's available
        private ByteBuffer readBuffer;
        private Selector selector;
        private boolean toExit;

        public DataShuffler() {

            readBuffer = ByteBuffer.allocate(8192 * 2);
        }

        private void initializeSelector() {
            selector = null;

            try {
                selector = Selector.open();

                for (Sym session : Bridge.this.sessions) {
                    session.getReceiver().getRtpDatagramChannel()
                            .configureBlocking(false);
                    session.getReceiver().getRtpDatagramChannel().register(
                            selector, SelectionKey.OP_READ);
                }
                initializeSelectors = false;

            } catch (ClosedChannelException ex) {
                logger.error("Unepxected exception", ex);
                return;
            } catch (IOException ex) {
                logger.error("Unepxected exception", ex);
                return;
            }
        }

        public void run() {

            // Wait for an event one of the registered channels
            logger.debug("Starting Shuffler");

            while (true) {
                try {
                    if (initializeSelectors) {
                        initializeSelector();
                    }
                    selector.select();
                    if (toExit)
                        break;

                    // Iterate over the set of keys for which events are
                    // available
                    Iterator<SelectionKey> selectedKeys = selector
                            .selectedKeys().iterator();
                    while (selectedKeys.hasNext()) {
                        SelectionKey key = (SelectionKey) selectedKeys.next();
                        selectedKeys.remove();

                        if (!key.isValid()) {
                            continue;
                        }
                        if (key.isReadable()) {
                            readBuffer.clear();
                            DatagramChannel datagramChannel = (DatagramChannel) key
                                    .channel();
                            datagramChannel.receive(readBuffer);
                            processingCount++;
                            if (getState() != BridgeState.RUNNING) {
                                if (logger.isDebugEnabled()) {
                                    logger
                                            .debug("RtpBridge:Discarding packet. Bridge state is "
                                                    + getState());
                                }
                                continue;
                            }

                            if (datagramChannel.socket().getPort() % 2 == 1) {
                                logger
                                        .debug("RtpBridge:Got an RTCP packet -- discarding");
                                continue;

                            }

                            try {

                                for (Sym rtpSession : sessions) {
                                    if (datagramChannel == rtpSession
                                            .getReceiver()
                                            .getRtpDatagramChannel()) {
                                        if (logger.isDebugEnabled()) {
                                            logger.debug("got something on "
                                                    + rtpSession.getReceiver()
                                                            .getIpAddress()
                                                    + ":"
                                                    + rtpSession.getReceiver()
                                                            .getPort());
                                        }
                                        /*
                                         * Set the remote port of the
                                         * transmitter side of the connection.
                                         * This allows for NAT reboots ( port
                                         * can change while in progress. This is
                                         * not relevant for the LAN side.
                                         */
                                        if (rtpSession.getTransmitter() != null
                                                && rtpSession.getTransmitter()
                                                        .isKeepaliveStarted()) {
                                            rtpSession.getTransmitter()
                                                    .setPort(
                                                            datagramChannel
                                                                    .socket()
                                                                    .getPort());
                                        }
                                        continue;
                                    }
                                    SymEndpoint writeChannel = rtpSession
                                            .getTransmitter();
                                    if (writeChannel == null)
                                        continue;

                                    try {
                                        /*
                                         * We need to rewrite and generate a new
                                         * RTP packet if and only if We have
                                         * started the early media thread. AND
                                         * the user has specified a given packet
                                         * for RTP keepalive payload. We do this
                                         * to keep sequence number continuity.
                                         */
                                        if (writeChannel.isKeepaliveStarted()
                                                && writeChannel
                                                        .getKeepalivePayload() != null) {
                                            RtpPacket rtpPacket = writeChannel
                                                    .createRtpPacket(readBuffer);

                                            if (!writeChannel.isOnHold()) {
                                                writeChannel.send(rtpPacket);
                                            } else {
                                                if (logger.isDebugEnabled()) {
                                                    logger
                                                            .debug("WriteChannel on hold."
                                                                    + writeChannel
                                                                            .getIpAddress()
                                                                    + ":"
                                                                    + writeChannel
                                                                            .getPort()
                                                                    + " Not forwarding");
                                                }

                                            }
                                        } else {
                                            /*
                                             * No need for header rewrite. Just
                                             * flip and push out.
                                             */
                                            if (!writeChannel.isOnHold()) {
                                                writeChannel
                                                        .send((ByteBuffer) readBuffer
                                                                .flip());
                                            } else {
                                                if (logger.isDebugEnabled()) {
                                                    logger
                                                            .debug("WriteChannel on hold."
                                                                    + writeChannel
                                                                            .getIpAddress()
                                                                    + ":"
                                                                    + writeChannel
                                                                            .getPort()
                                                                    + " Not forwarding");
                                                }
                                            }
                                        }

                                    } catch (Exception ex) {
                                        logger
                                                .error(
                                                        "Unexpected error shuffling bytes",
                                                        ex);
                                    }
                                }
                            } finally {

                            }

                        }
                    }
                } catch (Exception ex) {
                    logger.error("Unexpected exception occured", ex);
                    for (Sym rtpSession : sessions) {
                        rtpSession.close();
                    }
                    setState(BridgeState.TERMINATED);
                    return;
                }

            }

        }

        public void exit() {
            this.toExit = true;
            selector.wakeup();

        }

    }

    // /////////////////////////////////////////////////////////////////////////////////
    // Constructors.
    // /////////////////////////////////////////////////////////////////////////////////

    /**
     * Default constructor.
     */
    public Bridge(boolean maintainPortParity) {
        this.parityFlag  = maintainPortParity;
        id = "bridge:" + Long.toString(Math.abs(new Random().nextLong()));
    }

    /**
     * Constructor.
     * 
     * @param itspAccountInfo
     * @throws IOException
     */
    public Bridge(Request request) throws IOException {

        this(true);

        try {

            this.sessionDescription = SdpFactory.getInstance()
                    .createSessionDescription(
                            new String(request.getRawContent()));
        } catch (SdpParseException ex) {
            throw new IOException("Unable to parse SDP ");
        }
    }

    /**
     * Constructor when we are given a media map
     */
    public Bridge(Set<Sym> rtpSession) {
        this(true);
        this.sessions.addAll(rtpSession);
    }

    // ///////////////////////////////////////////////////////////////////////////////////////////
    // Public Methods.
    // ///////////////////////////////////////////////////////////////////////////////////////////

    /**
     * Get my Id.
     * 
     * @return myId
     */
    public String getId() {

        return this.id;
    }

    public void start() {
        if (started)
            return;
        else
            started = true;

        this.dataShuffler = new DataShuffler();
        this.dataShufflerThread = new Thread(dataShuffler);
        this.state = BridgeState.RUNNING;
        this.dataShufflerThread.start();

    }

    public void stop() {

        for (Sym rtpSession : this.sessions) {
            if (rtpSession.getReceiver() != null)
                rtpSession.close();
        }

        if (this.dataShuffler != null)
            this.dataShuffler.exit();
        this.state = BridgeState.TERMINATED;

    }

    public void pause() {

        if (this.state == BridgeState.PAUSED)
            return;
        else if (this.state == BridgeState.RUNNING) {
            this.state = BridgeState.PAUSED;
        } else {
            throw new IllegalStateException("Cannot pause bridge in "
                    + this.state);
        }

    }

    public void resume() {
        if (this.state == BridgeState.RUNNING)
            return;
        else if (this.state == BridgeState.PAUSED) {
            this.state = BridgeState.RUNNING;
        } else {
            throw new IllegalStateException(" Cannot resume bridge in "
                    + this.state);
        }
    }

    public void setEarlyMedia(boolean earlyMediaFlag) {
        this.earlyMedia = earlyMediaFlag;

    }

    public boolean isEarlyMedia() {
        return this.earlyMedia;
    }

    public void setState(BridgeState newState) {
        this.state = newState;

    }

    public BridgeState getState() {
        return this.state;
    }

    public void addSymSession(Sym rtpSession) {
        String key = rtpSession.getId();
        this.sessions.add(rtpSession);
        this.initializeSelectors = true;
        rtpSession.rtpBridge = this;
    }

    public void removeSymSession(Sym rtpSession) {
        logger.debug("RtpBridge: removing RtpSession " + rtpSession.getId());
        this.sessions.remove(rtpSession);
        this.initializeSelectors = true;
    }

    @Override
    public String toString() {
        StringBuffer retval = new StringBuffer();
        for (Sym rtpSession : this.sessions) {
            retval.append("{\n");
            retval.append(rtpSession.getId() + "\n");
            retval.append(rtpSession);
            retval.append("}\n");
        }
        return retval.toString();
    }

    public Set<Sym> getSessionTable() {
        return this.sessions;
    }

}
