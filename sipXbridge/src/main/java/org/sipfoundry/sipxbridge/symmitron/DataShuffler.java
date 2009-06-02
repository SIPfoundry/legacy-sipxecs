/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
/**
 * This is in charge of shuffling data. There is a single thread that shuffles data for all bridges.
 * 
 */
package org.sipfoundry.sipxbridge.symmitron;

import java.io.IOException;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.UnknownHostException;
import java.nio.ByteBuffer;
import java.nio.channels.ClosedChannelException;
import java.nio.channels.DatagramChannel;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.util.Iterator;
import java.util.Random;
import java.util.concurrent.atomic.AtomicBoolean;

import org.apache.log4j.Logger;

class DataShuffler implements Runnable {

    // The buffer into which we'll read data when it's available
    private static ByteBuffer readBuffer = ByteBuffer.allocate(8192);
    private static Selector selector;
    private static Logger logger = Logger.getLogger(DataShuffler.class.getPackage().getName());

    private static AtomicBoolean initializeSelectors = new AtomicBoolean(true);
    
    private static Random random = new Random();
    
    private static long packetCounter = Math.abs(random.nextLong());
    

    public DataShuffler() {

    }

    private static void initializeSelector() {

        try {
            if (selector != null) {
                selector.close();
            }
            selector = Selector.open();

            for (Bridge bridge : ConcurrentSet.getBridges()) {

                for (Sym session : bridge.sessions) {
                    try {
                        if (session.getReceiver() != null
                                && bridge.getState() == BridgeState.RUNNING
                                && session.getReceiver().getDatagramChannel().isOpen()) {

                            session.getReceiver().getDatagramChannel().configureBlocking(false);
                            session.getReceiver().getDatagramChannel().register(selector,
                                    SelectionKey.OP_READ);
                        }
                    } catch (ClosedChannelException ex) {
                        // Avoid loading any closed channels in our select set.
                        continue;
                    }
                }
                initializeSelectors.set(false);
            }

        } catch (IOException ex) {
            logger.error("Unepxected exception", ex);
            return;
        }

    }

    /**
     * Send method to send a packet received from a datagram channel to all the active legs of a
     * bridge.
     * 
     * <pre>
     * send(bridge,datagramChannel, addressWherePacketCameFrom) :
     *    for each sym in bridge do :
     *       if sym.receiver.datagramChannel == datagramChannel &amp;&amp; sym.isAutoLearn
     *           sym.receiver.farEnd = addressWherePacketCameFrom
     *       else if sym.transmitter.state == RUNNING :
     *          sym.transmitter.send(byteBuffer)
     * </pre>
     * 
     * @param bridge -- the bridge to forward through.
     * @param datagramChannel -- datagramChannel on which packet was received.
     * @param remoteAddress -- remote address to send to.
     * @throws UnknownHostException -- if there was a problem with the specified remote address.
     */
    public static void send(Bridge bridge, DatagramChannel datagramChannel,
            InetSocketAddress remoteAddress, long stamp, boolean selfRouted) throws UnknownHostException {
        try {

            if (logger.isTraceEnabled()) {
                logger.trace("BridgeSize = " + bridge.sessions.size());
            }

            for (Sym sym : bridge.sessions) {
                if (sym.getReceiver() != null
                        && datagramChannel == sym.getReceiver().getDatagramChannel()) {
                    if (logger.isTraceEnabled() && remoteAddress != null) {
                        logger.trace("remoteIpAddressAndPort : "
                                + remoteAddress.getAddress().getHostAddress() + ":"
                                + remoteAddress.getPort());

                    }
                    sym.lastPacketTime = System.currentTimeMillis();
                    sym.packetsReceived++;

                    bridge.setLastPacketTime(sym.lastPacketTime);

                    /*
                     * Set the remote port of the transmitter side of the connection. This allows
                     * for NAT reboots ( port can change while in progress. This is not relevant
                     * for the LAN side.
                     */
                    if (sym.getTransmitter() != null) {
                        AutoDiscoveryFlag autoDiscoveryFlag = sym.getTransmitter()
                                .getAutoDiscoveryFlag();

                        if (autoDiscoveryFlag != AutoDiscoveryFlag.NO_AUTO_DISCOVERY) {
                            
                            if (remoteAddress != null) {
                                // This packet was self routed.       
                                if (selfRouted ) {
                                    if ( sym.getTransmitter().getIpAddress() != null ) {
                                        continue;
                                    } else {
                                        String remoteHostAddress = remoteAddress.getAddress().getHostAddress();
                                        int remotePort = remoteAddress.getPort();
                              
                                        // This search is done just once on the first auto address discovery for a self
                                        // routed packet. Hence the loop is not too alarming subsequently, you dont ever have to look again.
                                        // However, there is probably a better way to do this.
                                        for ( Sym tsym : SymmitronServer.getSyms() ) {
                                            if (tsym.getTransmitter().getIpAddress().equals(remoteHostAddress) && 
                                                    tsym.getTransmitter().getPort() == remotePort ) {
                                                sym.getTransmitter().setIpAddressAndPort(tsym.getReceiver().getIpAddress(), tsym.getReceiver().getPort());
                                                break;
                                            }
                                        }
                                        if ( logger.isTraceEnabled()) {
                                            for ( Bridge br : SymmitronServer.getBridges() ) {
                                                logger.trace(br.toString());
                                            }
                                        }
                                    }
                            
                                } else {
                                    if (autoDiscoveryFlag == AutoDiscoveryFlag.IP_ADDRESS_AND_PORT) {
                                        sym.getTransmitter().setIpAddressAndPort(
                                                remoteAddress.getAddress().getHostAddress(),
                                                remoteAddress.getPort());
                                    } else if (autoDiscoveryFlag == AutoDiscoveryFlag.PORT_ONLY) {
                                        sym.getTransmitter().setPort(remoteAddress.getPort());
                                    }
                                }
                            }
                        }

                    }
                    continue;
                }
                SymTransmitterEndpoint writeChannel = sym.getTransmitter();
                if (writeChannel == null) {
                    continue;
                }

                try {

                    /*
                     * No need for header rewrite. Just duplicate, flip and push out. Important:
                     * We cannot do this outside the loop. See XECS-2425.
                     */
                    if (!writeChannel.isOnHold()) {
                        if (!sym.isVisited(stamp)) {
                            sym.setVisited(stamp);
                            ByteBuffer bufferToSend = readBuffer.duplicate();
                            bufferToSend.flip();
                            writeChannel.send((ByteBuffer) bufferToSend,stamp);
                            bridge.packetsSent++;
                            writeChannel.packetsSent++;
                        } else {
                            if (logger.isTraceEnabled()) {
                                logger.trace("sym " + sym + " Routing Loop detected!");
                            }
                        }
                    } else {
                        if (logger.isTraceEnabled()) {
                            logger.trace("WriteChannel on hold." + writeChannel.getIpAddress()
                                    + ":" + writeChannel.getPort() + " Not forwarding");
                        }
                    }

                } catch (Exception ex) {
                    logger.error("Unexpected error shuffling bytes", ex);
                }
            }
        } finally {

        }

    }

    /**
     * Sit in a loop running the following algorthim till exit:
     * 
     * <pre>
     * Let Si be a Sym belonging to Bridge B where an inbound packet P is received
     * Increment received packet count for B.
     * Record time for the received packet.
     * Record inboundAddress from where the packet was received
     * send(B,chan,inboundAddress)
     *            
     * </pre>
     * 
     */
    public void run() {

        // Wait for an event one of the registered channels
        logger.debug("Starting Shuffler");

        while (true) {
            Bridge bridge = null;
            try {

                if (initializeSelectors.get()) {
                    initializeSelector();
                }

                selector.select();

                // Iterate over the set of keys for which events are
                // available
                Iterator<SelectionKey> selectedKeys = selector.selectedKeys().iterator();
                while (selectedKeys.hasNext()) {
                    SelectionKey key = (SelectionKey) selectedKeys.next();
                    // The key must be removed or you can get one way audio ( i.e. will read a
                    // null byte ).
                    // (see issue 2075 ).
                    selectedKeys.remove();
                    if (!key.isValid()) {
                        if (logger.isDebugEnabled()) {
                            logger.debug("Discarding packet:Key not valid");
                        }

                        continue;
                    }
                    if (key.isReadable()) {
                        readBuffer.clear();
                        DatagramChannel datagramChannel = (DatagramChannel) key.channel();
                        if (!datagramChannel.isOpen()) {
                            if (logger.isDebugEnabled()) {
                                logger
                                    .debug("DataShuffler: Datagram channel is closed -- discarding packet.");
                            }
                            continue;
                        }
                        bridge = ConcurrentSet.getBridge(datagramChannel);
                        if (bridge == null) {
                            if (logger.isDebugEnabled()) {
                                logger
                                    .debug("DataShuffler: Discarding packet: Could not find bridge");
                            }
                            continue;
                        }
                        Sym packetReceivedSym = bridge.getReceiverSym(datagramChannel);
                        /*
                         * Note the original hold value and put the transmitter on which this packet was received on hold.
                         */
                        if ( packetReceivedSym == null  || packetReceivedSym.getTransmitter() == null ) {
                            if (logger.isDebugEnabled()) {
                                logger.debug("DataShuffler: Could not find sym for inbound channel -- discaring packet");
                            }
                            continue;
                        }
                        boolean holdValue = packetReceivedSym.getTransmitter().isOnHold();
                        packetReceivedSym.getTransmitter().setOnHold(true);
                       
                        InetSocketAddress remoteAddress = (InetSocketAddress) datagramChannel
                                .receive(readBuffer);

                        bridge.pakcetsReceived++;
                        if (bridge.getState() != BridgeState.RUNNING) {
                            if (logger.isDebugEnabled()) {
                                logger.debug("DataShuffler: Discarding packet: Bridge state is "
                                        + bridge.getState());
                            }
                            packetReceivedSym.getTransmitter().setOnHold(holdValue);
                            continue;
                        }
                        if (logger.isTraceEnabled()) {
                            logger.trace("got something on "
                                    + datagramChannel.socket().getLocalPort());
                        }

                        long stamp = getPacketCounter();
                        send(bridge, datagramChannel, remoteAddress,stamp,false);
                        /*
                         * Reset the old value.
                         */
                        packetReceivedSym.getTransmitter().setOnHold(holdValue);

                    }
                }
            } catch (Exception ex) {
                logger.error("Unexpected exception occured", ex);
                if (bridge != null && bridge.sessions != null) {
                    for (Sym rtpSession : bridge.sessions) {
                        rtpSession.close();
                    }
                }
                if (bridge != null)
                    bridge.setState(BridgeState.TERMINATED);
                continue;
            }

        }

    }

    public static void initializeSelectors() {
        initializeSelectors.set(true);
        if (selector != null) {
            selector.wakeup();
        }

    }

    /**
     * 
     * Implements the following search algorithm to retrieve a datagram channel that is associated
     * with the far end:
     * 
     * <pre>
     * getSelfRoutedDatagramChannel(farEnd)
     * For each selectable key do:
     *   let ipAddress be the local ip address
     *   let p be the local port
     *   let d be the datagramChannel associated with the key
     *   If  farEnd.ipAddress == ipAddress  &amp;&amp; port == localPort return d
     * return null
     * </pre>
     * 
     * @param farEnd
     * @return
     */

    public static DatagramChannel getSelfRoutedDatagramChannel(InetSocketAddress farEnd) {
        // Iterate over the set of keys for which events are
        // available
        InetAddress ipAddress = farEnd.getAddress();
        int port = farEnd.getPort();
        for (Iterator<SelectionKey> selectedKeys = selector.keys().iterator(); selectedKeys
                .hasNext();) {
            SelectionKey key = selectedKeys.next();
            if (!key.isValid()) {
                continue;
            }
            DatagramChannel datagramChannel = (DatagramChannel) key.channel();
            if (datagramChannel.socket().getLocalAddress().equals(ipAddress)
                    && datagramChannel.socket().getLocalPort() == port) {
                return datagramChannel;
            }

        }
        return null;

    }

    synchronized static long getPacketCounter() {
        long retval = packetCounter;
        packetCounter++;
        return retval;
    }

}
