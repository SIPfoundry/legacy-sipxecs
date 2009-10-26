/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxrelay;

import java.io.IOException;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.UnknownHostException;
import java.nio.ByteBuffer;
import java.nio.channels.ClosedChannelException;
import java.nio.channels.DatagramChannel;
import java.util.Random;
import java.util.TimerTask;
import java.util.concurrent.Semaphore;
import java.util.concurrent.atomic.AtomicLong;

import org.apache.log4j.Logger;

/**
 * Transmitter endpoint.
 * 
 * @author mranga
 * 
 */
final class SymTransmitterEndpoint extends SymEndpoint implements SymTransmitterEndpointInterface {

    private AtomicLong lastPacketSentTime = new AtomicLong(0);

    private ByteBuffer keepAliveBuffer = null;

    private ByteBuffer emptyBuffer = ByteBuffer.allocate(0);

    private AutoDiscoveryFlag remoteAddressAutoDiscovered = AutoDiscoveryFlag.NO_AUTO_DISCOVERY;

    KeepaliveMethod keepaliveMethod = KeepaliveMethod.NONE;

    private KeepaliveTimerTask keepaliveTimerTask;

    private static Logger logger = Logger.getLogger(SymTransmitterEndpoint.class.getPackage()
            .getName());

    protected boolean onHold = false;

    private boolean earlyMediaStarted = false;

    private int maxSilence;

    long packetsSent;

    private static final boolean checkForSelfRouting = true;

    private InetSocketAddress socketAddress;

    private InetSocketAddress farEnd;
    class SendKeepaliveWorkItem extends WorkItem {
         Bridge bridge;
      
        @Override
        public void doWork() {
            // TODO Auto-generated method stub
            long stamp = DataShuffler.getPacketCounter();
                 
            try {

                if (keepaliveMethod.equals(KeepaliveMethod.USE_EMPTY_PACKET)) {
                    if (datagramChannel != null && getSocketAddress() != null
                            && datagramChannel.isOpen() && getSocketAddress() != null) {
                        send(emptyBuffer,stamp);
                    }
                } else if (keepaliveMethod.equals(KeepaliveMethod.REPLAY_LAST_SENT_PACKET)
                        || keepaliveMethod.equals(KeepaliveMethod.USE_DUMMY_RTP_PAYLOAD)) {
                    if (keepAliveBuffer != null && datagramChannel != null
                            && getSocketAddress() != null && datagramChannel.isOpen()) {
                        logger.trace("Sending keepalive");
                        send((ByteBuffer) keepAliveBuffer,stamp);

                    }
                }
            } catch (ClosedChannelException ex) {
                /*
                 * This is not an error. Somebody closed socket. Just bail.
                 */
                logger.warn("Exiting early media thread due to closed channel", ex);
            } catch (Exception ex) {
                logger.error("Unexpected exception in sending early media ", ex);
                if (bridge != null) {
                    bridge.stop();
                }
            }
        }
        
    }
 
    /**
     * The keepalive timer task.
     * 
     */

    class KeepaliveTimerTask extends TimerTask {

        KeepaliveTimerTask() {

            logger.debug("Starting early media sender TimerTask : remote endpoint = "
                    + getIpAddress() + "/ " + getPort());

        }

        public void run() {

            if (datagramChannel == null) {
                return;
            }

            Bridge bridge = ConcurrentSet.getBridge(datagramChannel);
            if (bridge == null || bridge.getState() == BridgeState.TERMINATED || bridge.getState() == BridgeState.INITIAL) {
                if (logger.isDebugEnabled() && bridge != null) {
                    logger.debug("Bridge is not in running state " + bridge.getState());
                }
                return;
            }
            
            /*
             * Wait till we have a remote socket address and transmit
             * as soon as we see it.
             */

            if (getSocketAddress() == null) {
                while (getSocketAddress() == null) {
                    try {
                        Thread.sleep(20);
                    } catch (Exception ex) {
                    }
                }
                
            }

            long now = System.currentTimeMillis();

            if (now - bridge.getLastPacketTime() < maxSilence) {
                return;
            }
            
            SendKeepaliveWorkItem workItem = new SendKeepaliveWorkItem();
            workItem.bridge = bridge;
            DataShuffler.addWorkItem(workItem);

           

        }

    }

    private void setFarEnd() {
        try {
            InetAddress remoteAddress = this.getSocketAddress().getAddress();
            int remotePort = this.getSocketAddress().getPort();
            InetAddress farEndAddress = (SymmitronServer.getPublicInetAddress() != null
                    && remoteAddress.equals(SymmitronServer.getPublicInetAddress()) ? SymmitronServer
                    .getLocalInetAddress()
                    : remoteAddress);
            this.farEnd = new InetSocketAddress(farEndAddress, remotePort);
        } catch (Exception ex) {
            logger.error("Unexpected exception in setting far end address", ex);

        }

    }

    private static boolean isPacketSelfRouted(InetSocketAddress destinationSockaddr) {
        try {
            int port = destinationSockaddr.getPort();
            InetAddress destination = destinationSockaddr.getAddress();
            if (port >= SymmitronServer.getRangeLowerBound()
                    && port < SymmitronServer.getRangeUpperBound()) {
                if (!destination.equals(SymmitronServer.getLocalInetAddress())) {
                    return false;
                } else {
                    logger.trace("Self routed!");
                    return true;
                }
            } else {
                return false;
            }
        } catch (Exception ex) {
            logger.error("Unexpected exception", ex);
            throw new SymmitronException("Unexpected exception ", ex);
        }

    }

    private void startKeepaliveTimer() {

        if (earlyMediaStarted) {
            return;
        }
        logger.debug("startEarlyMediaThread " + this.toString());
        this.earlyMediaStarted = true;
        this.keepaliveTimerTask = new KeepaliveTimerTask();
        SymmitronServer.timer.schedule(this.keepaliveTimerTask, 0, this.maxSilence);

    }

    public SymTransmitterEndpoint() {
        super();
    }

    /**
     * @return the earlyMediaStarted
     */
    public boolean isKeepaliveStarted() {
        return earlyMediaStarted;
    }

    /**
     * The transmitter send method. Implements the following algorithm:
     * 
     * <pre>
     * SymTransmitterEndpoint::send(byteBuffer) :
     *    record last packet sent time
     *    if we have a remote address recorded for this transmitter and packet is
     *       potentially self routed then :
     *          channel = getSelfRoutedDatagramChannel(remoteAddress)
     *          if channel != null :
     *              bridge = getBridge(channel)
     *              DataShuffler.send(bridge,channel,remoteAddress)
     *          else:
     *              this.datagramChannel.send(byteBuffer,remoteAddress)
     *    else if we have a remote address recorded for this transmitter:
     *         this.datagramChannel.send(byeBuffer,remoteAddress)
     *         
     * </pre>
     * 
     * @param byteBuffer
     * @throws IOException
     */
    public void send(ByteBuffer byteBuffer, long stamp) throws IOException {

        if (this.getSocketAddress() == null) {
            if (logger.isTraceEnabled()) {
                logger.trace("Cannot send -- remote address is null!");
            }
            return;
        }

        /*
         * Record the time when packet is sent out.
         */
        this.lastPacketSentTime.set(System.currentTimeMillis());

       
        
        try {

            /*
             * Check if the packet is self-routed. If so route it back.
             */
            if (checkForSelfRouting && this.farEnd != null && isPacketSelfRouted(this.farEnd)) {
                if (logger.isTraceEnabled()) {
                    logger.trace("SymTransmitterEndpoint:remoteAddress = " + this.farEnd);
                }

                DatagramChannel channel = DataShuffler.getSelfRoutedDatagramChannel(this.farEnd);

                if (channel != null) {
                    if (logger.isTraceEnabled()) {
                        logger.trace("SymTransmitterEndpoint:selfRoutedDatagramChannel = "
                                + channel);
                    }
                    Bridge bridge = ConcurrentSet.getBridge(channel);
                    if (logger.isTraceEnabled()) {
                        logger.trace("SymTransmitterEndpoint:selfRoutedBridge = " + bridge);
                    }
                    if (bridge != null) {
                        DataShuffler.send(bridge, channel, this.farEnd,stamp,true);
                        return;
                    }
                } else {
                    logger.trace("Could not find self routed datagram channel");
                }
            }

            if (this.datagramChannel != null && this.getSocketAddress() != null ) {
                int bytesSent = this.datagramChannel.send(byteBuffer, this.getSocketAddress());
                if (logger.isTraceEnabled()) {
                    logger.trace("SymTransmitterEndpoint:actually sending to "
                            + this.getSocketAddress() + " sent " + bytesSent + " bytes ");
                }

            }
            if (keepaliveMethod.equals(KeepaliveMethod.REPLAY_LAST_SENT_PACKET)) {
                this.keepAliveBuffer = byteBuffer;
            }

        } finally {
            if ( logger.isTraceEnabled() ) {
                 logger.trace("Done sending packet on " + this.getSym().getId());
            }
        }
    }

    public void setMaxSilence(int maxSilence, KeepaliveMethod keepaliveMethod) {
        if (logger.isDebugEnabled()) {
            logger.debug("RtpEndpoint : setMaxSilence " + maxSilence + " keepaliveMethod = "
                    + keepaliveMethod);
        }
        if (this.earlyMediaStarted) {
            logger.debug("early media started !");
            return;
        }
        this.maxSilence = maxSilence;
        this.keepaliveMethod = keepaliveMethod;
        if (keepaliveMethod == KeepaliveMethod.USE_DUMMY_RTP_PAYLOAD
                || keepaliveMethod == KeepaliveMethod.REPLAY_LAST_SENT_PACKET) {
            this.keepAliveBuffer = DummyRtpPacket.createDummyRtpPacket();
        }
        if (maxSilence != 0 && !keepaliveMethod.equals(KeepaliveMethod.NONE)) {
            this.startKeepaliveTimer();
        }
    }

    public void stopKeepalive() {
        if (this.keepaliveTimerTask != null)
            this.keepaliveTimerTask.cancel();

    }

    public void setAutoDiscoveryFlag(AutoDiscoveryFlag autoDiscoveryFlag) {
        this.remoteAddressAutoDiscovered = autoDiscoveryFlag;
       

    }

    public void setOnHold(boolean onHold) {
        this.onHold = onHold;

    }

    public boolean isOnHold() {
        return this.onHold;
    }

    /**
     * Set the remote IP address and port.
     * 
     * @param ipAddress
     * @param port
     * @throws UnknownHostException
     */
    public void setIpAddressAndPort(String ipAddress, int port) throws UnknownHostException {
        super.setIpAddress(ipAddress);
        super.setPort(port);
        // Note that the IP address does not need to be specified ( can be auto
        // discovered ).
        if (ipAddress != null && port != 0) {
            setSocketAddress(new InetSocketAddress(InetAddress.getByName(ipAddress), port));

        } else {
            setSocketAddress(null);
        }
    }

    @Override
    public void setPort(int port) throws IllegalArgumentException {
        try {
            super.setPort(port);
            if (getIpAddress() != null && port != 0) {
                setSocketAddress(new InetSocketAddress(InetAddress.getByName(getIpAddress()), port));

            } else {
                setSocketAddress(null);
            }
        } catch (UnknownHostException ex) {
            logger.error("Error processing request" ,ex);
            throw new IllegalArgumentException("Unknown host " );
        }
    }

    @Override
    public void setIpAddress(String ipAddress) throws IllegalArgumentException {
        try {
            super.setIpAddress(ipAddress);
            if (ipAddress != null && getPort() != 0) {
                setSocketAddress(new InetSocketAddress(InetAddress.getByName(ipAddress), getPort()));

            } else {
                setSocketAddress(null);
            }
        } catch (UnknownHostException ex) {
            logger.error("Error processing request ", ex);
            throw new IllegalArgumentException("Unknown host " + ipAddress);
        }
    }

    /**
     * @return the remoteAddressAutoDiscovered
     */
    public AutoDiscoveryFlag getAutoDiscoveryFlag() {
        return remoteAddressAutoDiscovered;
    }

    /**
     * @param packetsSent the packetsSent to set
     */
    public void setPacketsSent(long packetsSent) {
        this.packetsSent = packetsSent;
    }

    /**
     * @return the packetsSent
     */
    public long getPacketsSent() {
        return packetsSent;
    }

    /**
     * @param socketAddress the socketAddress to set
     */
    protected void setSocketAddress(InetSocketAddress socketAddress) {
        this.socketAddress = socketAddress;
        if (socketAddress != null) {
            this.setFarEnd();
        }

    }

    public void connect() throws IOException {
        /*
         * if (this.datagramChannel.isConnected()) { this.datagramChannel.disconnect(); }
         * this.datagramChannel.connect(socketAddress);
         */

    }

    /**
     * @return the socketAddress
     */
    public InetSocketAddress getSocketAddress() {
        return socketAddress;
    }

}
