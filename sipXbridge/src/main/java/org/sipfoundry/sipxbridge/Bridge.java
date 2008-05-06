/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxbridge;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.net.SocketAddress;
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

    private ConcurrentSet sessions = new ConcurrentSet(this);

    private boolean started;

    private Thread dataShufflerThread;

    private DataShuffler dataShuffler;

    private BridgeState state = BridgeState.INITIAL;

    SessionDescription sessionDescription;

    boolean initializeSelectors = true;

    private String id;

    private long processingCount = 0;

    private long creationTime = System.currentTimeMillis();

    private long lastPacketTime;

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
                    session.getReceiver().getDatagramChannel()
                            .configureBlocking(false);
                    session.getReceiver().getDatagramChannel().register(
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
                            InetSocketAddress remoteAddress = (InetSocketAddress) datagramChannel.receive(readBuffer);
                            
                            processingCount++;
                            if (getState() != BridgeState.RUNNING) {
                                if (logger.isDebugEnabled()) {
                                    logger
                                            .debug("RtpBridge:Discarding packet. Bridge state is "
                                                    + getState());
                                }
                                continue;
                            }

                            try {

                                for (Sym sym : sessions) {
                                    if (datagramChannel == sym.getReceiver()
                                            .getDatagramChannel()) {
                                        if (logger.isDebugEnabled()) {
                                            logger.debug("got something on "
                                                    + sym.getReceiver()
                                                            .getIpAddress()
                                                    + ":"
                                                    + sym.getReceiver()
                                                            .getPort());
                                            logger.debug("remoteIpAddressAndPort : " + remoteAddress.getHostName() + ":" + remoteAddress.getPort());
                                        }
                                        sym.lastPacketTime = System
                                                .currentTimeMillis();
                                        sym.packetsReceived ++;
                                        
                                        Bridge.this.lastPacketTime = sym.lastPacketTime;

                                        /*
                                         * Set the remote port of the
                                         * transmitter side of the connection.
                                         * This allows for NAT reboots ( port
                                         * can change while in progress. This is
                                         * not relevant for the LAN side.
                                         */

                                        if (sym.getTransmitter() != null
                                                && sym.getTransmitter().isRemoteAddressAutoDiscovered()) {
                                           
                                            sym.getTransmitter().setIpAddressAndPort(remoteAddress.getHostName(), remoteAddress.getPort());
                                            
                                        }

                                        continue;
                                    }
                                    SymTransmitterEndpoint writeChannel = sym
                                            .getTransmitter();
                                    if (writeChannel == null)
                                        continue;

                                    try {

                                        /*
                                         * No need for header rewrite. Just flip
                                         * and push out.
                                         */
                                        if (!writeChannel.isOnHold()) {
                                            writeChannel
                                                    .send((ByteBuffer) readBuffer
                                                            .flip());
                                            sym.getTransmitter().packetsSent ++;
                                           
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

    // ///////////////////////////////////////////////////////////////////////////////////
    // Private methods.
    // ///////////////////////////////////////////////////////////////////////////////////
    void setState(BridgeState newState) {
        this.state = newState;

    }

    // /////////////////////////////////////////////////////////////////////////////////
    // Constructors.
    // /////////////////////////////////////////////////////////////////////////////////

    /**
     * Default constructor.
     */
    public Bridge() {
        id = "bridge:" + Long.toString(Math.abs(new Random().nextLong()));
    }

    /**
     * Constructor.
     * 
     * @param itspAccountInfo
     * @throws IOException
     */
    public Bridge(Request request) throws IOException {

        this();

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
        this();
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

    /**
     * Starts the data shuffler running
     * 
     * 
     */

    public void start() throws IllegalStateException {
        if (started)
            return;
        else
            started = true;

        this.dataShuffler = new DataShuffler();
        this.dataShufflerThread = new Thread(dataShuffler);
        this.state = BridgeState.RUNNING;
        this.dataShufflerThread.start();

    }

    /**
     * Stops the data shuffler from running and kills all syms.
     * 
     */
    public void stop() {

        for (Sym rtpSession : this.sessions) {
            if (rtpSession.getReceiver() != null)
                rtpSession.close();
        }

        if (this.dataShuffler != null)
            this.dataShuffler.exit();
        this.state = BridgeState.TERMINATED;

    }

    /**
     * Pauses the bridge.
     * 
     * 
     */
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

    /**
     * Resumes the operation of the bridge.
     * 
     * 
     */

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

    /**
     * Gets the current state.
     * 
     * @return
     */
    public BridgeState getState() {
        return this.state;
    }

    /**
     * Add a sym to this bridge.
     * 
     * @param sym
     */
    public void addSym(Sym sym) {
        this.sessions.add(sym);
        this.initializeSelectors = true;
        sym.setBridge(this);
        if (logger.isDebugEnabled()) {
            logger.debug("addSymSession : " + sym);
            logger.debug("addSymSession : " + this);
        }
    }

    /**
     * Remove a Sym from this bridge.
     * 
     * @param rtpSession
     */
    public void removeSym(Sym rtpSession) {
        logger.debug("RtpBridge: removing RtpSession " + rtpSession.getId());
        this.sessions.remove(rtpSession);
        this.initializeSelectors = true;
        if (logger.isDebugEnabled()) {
            logger.debug("removeSymSession : " + rtpSession);
            logger.debug("removeSymSession : " + this);
        }
    }

    public Set<Sym> getSyms() {
        return this.sessions;
    }

    public long getLastPacketTime() {
        return this.lastPacketTime;
    }

    public long getCreationTime() {
        return this.creationTime;
    }

    @Override
    public String toString() {
        StringBuffer retval = new StringBuffer();
        retval.append("Bridge = [ \n");
        retval.append("id = " + this.getId() + "\n");
        for (Sym rtpSession : this.sessions) {
            retval.append("{\n");
            retval.append(rtpSession);
            retval.append("}\n");
        }
        retval.append(" ] \n");
        return retval.toString();
    }

}
