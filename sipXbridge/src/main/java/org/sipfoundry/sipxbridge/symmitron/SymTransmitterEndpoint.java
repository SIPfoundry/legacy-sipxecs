/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxbridge.symmitron;

import java.io.IOException;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.UnknownHostException;
import java.nio.ByteBuffer;
import java.nio.channels.ClosedChannelException;
import java.util.TimerTask;

import org.apache.log4j.Logger;

/**
 * Transmitter endpoint.
 * 
 * @author mranga
 * 
 */
public class SymTransmitterEndpoint extends SymEndpoint {

    private long lastPacketSentTime;

    private transient byte[] keepalivePayload = null;

    private ByteBuffer keepAliveBuffer = null;

    private ByteBuffer emptyBuffer = ByteBuffer.allocate(0);

    private AutoDiscoveryFlag remoteAddressAutoDiscovered = AutoDiscoveryFlag.NO_AUTO_DISCOVERY;

    private String keepaliveMethod = "USE-EMPTY-PACKET";

    private KeepaliveTimerTask keepaliveTimerTask;

    private static Logger logger = Logger.getLogger(SymTransmitterEndpoint.class);

    protected boolean onHold = false;

    private boolean earlyMediaStarted = false;

    private int maxSilence;

    long packetsSent;

    // private InetAddress inetAddress;

    private InetSocketAddress socketAddress;

    /**
     * The keepalive timer task.
     * 
     */

    class KeepaliveTimerTask extends TimerTask {

        KeepaliveTimerTask() {

            logger.debug("Starting early media sender TimerTask : remote endpoint = "
                    + getIpAddress() + "/ " + port);

        }

        public void run() {
            try {

                if (datagramChannel == null)
                    return;
                long now = System.currentTimeMillis();
                if (now - lastPacketSentTime < maxSilence) {
                    return;
                }
                if (keepaliveMethod.equals("USE-EMPTY-PACKET")) {
                    if (datagramChannel != null && getSocketAddress() != null
                            && datagramChannel.isOpen() && getSocketAddress() != null)
                        datagramChannel.send(emptyBuffer, getSocketAddress());

                } else if (keepaliveMethod.equals("REPLAY-LAST-SENT-PACKET")) {
                    if (keepAliveBuffer != null && datagramChannel != null
                            && getSocketAddress() != null && datagramChannel.isOpen()
                            && getSocketAddress() != null) {
                        datagramChannel.send((ByteBuffer) keepAliveBuffer, getSocketAddress());

                    }
                }
            } catch (ClosedChannelException ex) {
                logger.warn("Exitting early media thread due to closed channel", ex);
                this.cancel();

            } catch (Exception ex) {
                logger.fatal("Unexpected exception in sending early media ", ex);
                this.cancel();
            }

        }

    }

    private void startKeepaliveTimer() {

        logger.debug("startEarlyMediaThread " + this.toString());
        if (earlyMediaStarted)
            return;
        this.earlyMediaStarted = true;
        this.keepaliveTimerTask = new KeepaliveTimerTask();
        SymmitronServer.timer.schedule(this.keepaliveTimerTask, this.maxSilence / 2,
                this.maxSilence);

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

    public void send(ByteBuffer byteBuffer) throws IOException {

        if (logger.isDebugEnabled())
            logger.debug("Sending to " + this.getSocketAddress());
        this.lastPacketSentTime = System.currentTimeMillis();

        if (this.datagramChannel != null) {
            this.datagramChannel.send(byteBuffer, this.getSocketAddress());
        }
        if (keepaliveMethod.equals("REPLAY-LAST-SENT-PACKET")) {
            this.keepAliveBuffer = byteBuffer;
        }
    }

    public void setMaxSilence(int maxSilence, String keepaliveMethod) {
        logger.debug("RtpEndpoint : setMaxSilence " + maxSilence);
        if (this.earlyMediaStarted) {
            logger.debug("early media started !");
            return;
        }
        this.maxSilence = maxSilence;
        this.keepaliveMethod = keepaliveMethod;
        if (maxSilence != 0 && !keepaliveMethod.equals("NONE")) {
            this.startKeepaliveTimer();
        }
    }

    public void setKeepalivePayload(byte[] keepAlivePacketData) {
        this.keepalivePayload = keepAlivePacketData;

    }

    public byte[] getKeepalivePayload() {
        return this.keepalivePayload;
    }

    public void stopKeepalive() {
        if (this.keepaliveTimerTask != null)
            this.keepaliveTimerTask.cancel();

    }

    void computeAutoDiscoveryFlag() throws Exception {
        if (this.ipAddress == null && this.port == 0) {
            this.remoteAddressAutoDiscovered = AutoDiscoveryFlag.IP_ADDRESS_AND_PORT;
        } else if (this.ipAddress != null && this.port == 0) {
            this.remoteAddressAutoDiscovered = AutoDiscoveryFlag.PORT_ONLY;
        } else if (this.ipAddress != null && this.port != 0) {
            this.remoteAddressAutoDiscovered = AutoDiscoveryFlag.NO_AUTO_DISCOVERY;
        }

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
        super.ipAddress = ipAddress;
        super.port = port;
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
            if (ipAddress != null && port != 0) {
                setSocketAddress(new InetSocketAddress(InetAddress.getByName(ipAddress), port));

            } else {
                setSocketAddress(null);
            }
        } catch (UnknownHostException ex) {
            throw new IllegalArgumentException("Unknown host " + ipAddress);
        }
    }

    @Override
    public void setIpAddress(String ipAddress) throws IllegalArgumentException {
        try {
            super.setIpAddress(ipAddress);
            if (ipAddress != null && port != 0) {
                setSocketAddress(new InetSocketAddress(InetAddress.getByName(ipAddress), port));

            } else {
                setSocketAddress(null);
            }
        } catch (UnknownHostException ex) {
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

    }

    public void connect() throws IOException {
       /*
       if (this.datagramChannel.isConnected()) {
            this.datagramChannel.disconnect();
        }
        this.datagramChannel.connect(socketAddress);*/

    }

    /**
     * @return the socketAddress
     */
    public InetSocketAddress getSocketAddress() {
        return socketAddress;
    }

}
