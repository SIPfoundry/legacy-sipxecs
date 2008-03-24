/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxbridge;

import java.io.IOException;
import java.net.DatagramPacket;
import java.nio.ByteBuffer;
import java.nio.channels.ClosedChannelException;
import java.nio.channels.DatagramChannel;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.nio.channels.spi.SelectorProvider;
import java.util.HashSet;
import java.util.Hashtable;
import java.util.Iterator;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

import javax.sdp.SdpFactory;
import javax.sdp.SdpParseException;
import javax.sdp.SessionDescription;
import javax.sip.Dialog;
import javax.sip.SipProvider;
import javax.sip.message.Request;

import org.apache.log4j.Logger;
import org.apache.log4j.Priority;

/**
 * Manages the pairing of Rtp pipes. One side belongs to the Lan the other side
 * terminates the ITSP. There is a data shuffler that reads from one end and
 * writes to the other side of the bridge. This method can potentially benifit
 * from a kernel level copy such as ip-tables.
 * 
 * @author M. Ranganathan
 * 
 */
public class RtpBridge {
    private static Logger logger = Logger.getLogger(RtpBridge.class);

    private ConcurrentHashMap<Dialog, RtpSession> rtpSessions = new ConcurrentHashMap<Dialog, RtpSession>();

    private boolean started;

    private Thread dataShufflerThread;

    private DataShuffler dataShuffler;

    private boolean earlyMedia = true;

    private RtpBridgeState state = RtpBridgeState.INITIAL;

    private ItspAccountInfo itspAccountInfo;

    private SessionDescription sessionDescription;

    boolean initializeSelectors = true;

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

                for (RtpSession rtpSession : RtpBridge.this.rtpSessions
                        .values()) {
                    rtpSession.getReceiver().getRtpDatagramChannel()
                            .configureBlocking(false);
                    rtpSession.getReceiver().getRtpDatagramChannel().register(
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
                            if (getState() != RtpBridgeState.RUNNING) {
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

                            /*
                             * We need to form a set here because there could be
                             * several dialogs all referencing the same
                             * RtpSession. The values() operator returns a
                             * collection (multiset) of RtpSessions. If this is
                             * used, we wind up sending the same rtp packet to
                             * the destination multiple times.
                             */
                            HashSet<RtpSession> valueSet = new HashSet<RtpSession>();
                            valueSet
                                    .addAll(RtpBridge.this.rtpSessions.values());

                            for (RtpSession rtpSession : valueSet) {
                                if (datagramChannel == rtpSession.getReceiver()
                                        .getRtpDatagramChannel()) {
                                    if (logger.isDebugEnabled()) {
                                        logger.debug("got something on "
                                                + rtpSession.getReceiver()
                                                        .getIpAddress()
                                                + ":"
                                                + rtpSession.getReceiver()
                                                        .getPort());
                                    }
                                    continue;
                                }
                                RtpEndpoint writeChannel = rtpSession
                                        .getTransmitter();

                                try {
                                    /*
                                     * We need to rewrite if and only if We have
                                     * started the early media thread. This
                                     * applies for packets outbound through the
                                     * NAT.
                                     */
                                    if (writeChannel.isEarlyMediaStarted()) {
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
                                        /* No need for header rewrite */
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
                                    logger.error(
                                            "Unexpected error shuffling bytes",
                                            ex);
                                }
                            }

                        }
                    }
                } catch (Exception ex) {
                    logger.error("Unexpected exception occured", ex);
                    for (RtpSession rtpSession : rtpSessions.values()) {
                        rtpSession.close();
                    }
                    setState(RtpBridgeState.TERMINATED);
                    return;
                }

            }

        }

        public void exit() {
            this.toExit = true;
            selector.wakeup();

        }

    }

    /**
     * Constructor.
     * 
     * @param itspAccountInfo
     * @throws IOException
     */
    public RtpBridge(Request request, ItspAccountInfo itspAccountInfo)
            throws IOException {
        try {
            this.itspAccountInfo = itspAccountInfo;
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
    public RtpBridge(ConcurrentHashMap<Dialog, RtpSession> rtpSessions) {
        this.rtpSessions = rtpSessions;
    }

    /*
     * Creates a media endpoint and returns a session description for that media
     * endpoint.
     */
    private static RtpEndpoint createRtpEndpoint(boolean useGlobalAddress) {
        try {

            RtpEndpoint mediaEndpoint = new RtpEndpoint(false, useGlobalAddress);

            return mediaEndpoint;

        } catch (Exception ex) {
            String s = "Unexpected exception";
            logger.fatal("could not create media endpoint", ex);
            throw new RuntimeException(s, ex);

        }
    }

    /**
     * RTP session that is connected to the WAN Side.
     * 
     * @return
     */
    public RtpSession getWanRtpSession(Dialog dialog) {
        try {
            RtpSession rtpSession = this.rtpSessions.get(dialog);
            if (rtpSession == null) {
                rtpSession = new RtpSession(this);

                RtpEndpoint mediaEndpoint = createRtpEndpoint(itspAccountInfo
                        .isGlobalAddressingUsed());
                rtpSession.setMyEndpoint(mediaEndpoint);
                SessionDescription sd = SdpFactory.getInstance()
                        .createSessionDescription(
                                this.sessionDescription.toString());
                rtpSession.getReceiver().setSessionDescription(sd);
                this.rtpSessions.put(dialog, rtpSession);
            }
            return rtpSession;
        } catch (SdpParseException ex) {
            throw new RuntimeException("Unexpected exception -- FIXME", ex);
        }

    }

    /**
     * Get the rtp session for a given Dialog.
     * 
     */
    public RtpSession getRtpSession(Dialog dialog) {

        return this.rtpSessions.get(dialog);

    }

    /**
     * RTP session that is connected to the lAN side.
     * 
     * @return
     */
    public RtpSession getLanRtpSession(Dialog dialog) {
        try {
            RtpSession rtpSession = this.rtpSessions.get(dialog);
            if (rtpSession == null) {
                rtpSession = new RtpSession(this);
                RtpEndpoint endpoint = createRtpEndpoint(false);
                rtpSession.setMyEndpoint(endpoint);
                SessionDescription sd = SdpFactory.getInstance()
                        .createSessionDescription(
                                this.sessionDescription.toString());
                rtpSession.getReceiver().setSessionDescription(sd);
                this.rtpSessions.put(dialog, rtpSession);

            }
            return this.rtpSessions.get(dialog);
        } catch (SdpParseException ex) {
            throw new RuntimeException("Unexpected exception -- FIXME", ex);
        }
    }

    public void start() {
        if (started)
            return;
        else
            started = true;

        this.dataShuffler = new DataShuffler();
        this.dataShufflerThread = new Thread(dataShuffler);
        this.dataShufflerThread.start();
        this.state = RtpBridgeState.RUNNING;

    }

    public void stop() {

        for (RtpSession rtpSession : this.rtpSessions.values()) {
            if (rtpSession.getReceiver() != null)
                rtpSession.close();
        }

        if (this.dataShuffler != null)
            this.dataShuffler.exit();
        this.state = RtpBridgeState.TERMINATED;

    }

    public void pause() {

        if (this.state == RtpBridgeState.PAUSED)
            return;
        else if (this.state == RtpBridgeState.RUNNING) {
            this.state = RtpBridgeState.PAUSED;
        } else {
            throw new IllegalStateException("Cannot pause bridge in "
                    + this.state);
        }

    }

    public void resume() {
        if (this.state == RtpBridgeState.RUNNING)
            return;
        else if (this.state == RtpBridgeState.PAUSED) {
            this.state = RtpBridgeState.RUNNING;
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

    public void setState(RtpBridgeState newState) {
        this.state = newState;

    }

    public RtpBridgeState getState() {
        return this.state;
    }

    public void setRtpSession(Dialog dialog, RtpSession rtpSession) {
        this.rtpSessions.put(dialog, rtpSession);
    }

    public void removeDialog(Dialog dialog) {
        logger.debug("RtpBridge: removing dialog " + dialog);
        // this.rtpSessions.remove(dialog);

    }

    @Override
    public String toString() {
        StringBuffer retval = new StringBuffer();
        for (Dialog dialog : this.rtpSessions.keySet()) {
            RtpSession rtpSession = this.rtpSessions.get(dialog);
            retval.append("{\n");
            retval.append(dialog.getDialogId() + "\n");
            retval.append(dialog + "\n");
            retval.append(rtpSession);
            retval.append("}\n");
        }
        return retval.toString();
    }

    public Map<Dialog, RtpSession> getRtpSessionTable() {
        return this.rtpSessions;
    }

}
