/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxrelay;

import java.io.Serializable;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Random;

import org.apache.log4j.Logger;
import org.apache.xmlrpc.XmlRpcException;

/**
 * Representation of a media session. A media sesison is a pair of media endpoints.
 * 
 * @author M. Ranganathan
 * 
 */
final class Sym implements SymInterface, Serializable {

    private static final long serialVersionUID = -2762988912663901623L;

    private static Logger logger = Logger.getLogger(Sym.class.getPackage().getName());

    private String id;

    private long creationTime;

    long lastPacketTime;

    private int inactivityTimeout = -1;

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

    long packetsReceived;
    
    private long visited;
    
    private HashMap<String,Integer> strayPacketCounters = new HashMap<String,Integer>();
    
    private static HashSet<String> strayPacketAlarmTable = new HashSet<String>();

    public Sym() {
        id = "sym:" + Math.abs(new Random().nextLong());
        this.creationTime = System.currentTimeMillis();

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
    protected void setReceiver(SymReceiverEndpoint receiverEndpoint) {
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
     * Set the remote endpoint and connect the datagram socket to the remote socket address.
     * 
     * @param hisEndpoint
     */
    protected void setTransmitter(SymTransmitterEndpoint hisEndpoint) {

        this.transmitter = hisEndpoint;
        hisEndpoint.setSym(this);
        if ( this.receiver != null ) {
            this.transmitter.datagramChannel = this.receiver.datagramChannel;
        } else {
            throw new IllegalStateException ("Attempt to set transmitter but Receiver not set.");
        }
        if ( logger.isDebugEnabled() ) {
            logger.debug("Sym:setTransmitter : " + this.toString());
        }

    }

    /**
     * Get the transmitter endpoint.
     * 
     */
    public SymTransmitterEndpoint getTransmitter() {
        return transmitter;
    }

    public void close() {
    	if ( logger.isDebugEnabled()) {
    		logger.debug("Closing channel : " + this.getId());
    	}
        try {
            if (this.receiver != null) {
                if (this.receiver.getDatagramChannel() != null) {
                    this.receiver.getDatagramChannel().socket().close();
                    this.receiver.getDatagramChannel().close();
                    ConcurrentSet.removeChannel(this.receiver.getDatagramChannel());
                } else {
                    logger.debug("receiver datagram channel is null");
                }
                int port = this.receiver.getPort();
                PortRange portRange = new PortRange(port, port + 1);
                SymmitronServer.getPortManager().free(portRange);
                this.receiver = null;

            }
            if (this.transmitter != null) {
                if (this.transmitter.getDatagramChannel() != null) {
                    this.transmitter.getDatagramChannel().socket().close();
                    this.transmitter.getDatagramChannel().close();
                    ConcurrentSet.removeChannel(this.transmitter.getDatagramChannel());
                } else {
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
            sbuf.append(" RECEIVER : " + this.getReceiver().getIpAddress() + ":"
                    + this.getReceiver().getPort());
        } else {
            sbuf.append("NO RECEIVER");
        }

        if (this.transmitter != null) {
            sbuf.append(": TRANSMITTER : " + this.getTransmitter().getIpAddress() + ":"
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
     * @param rtpBridge the rtpBridge to set
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

    public void setInactivityTimeout(int inactivityTimeout) {
        if (inactivityTimeout > 0) {
            this.inactivityTimeout = inactivityTimeout;
        }
    }

    public boolean isTimedOut() {
        if (this.inactivityTimeout == -1)
            return false;
        long time = System.currentTimeMillis();
        return time - lastPacketTime > this.inactivityTimeout;
    }

    public long getPacketsReceived() {

        return this.packetsReceived;
    }

    public Map<String, Object> getStats() {
        Map<String, Object> retval = new HashMap<String, Object>();
      
        retval.put(Symmitron.SESSION_STATE, this.getState().toString());
        retval.put(Symmitron.CREATION_TIME, new Long(this.getCreationTime()).toString());
        retval.put(Symmitron.LAST_PACKET_RECEIVED, new Long(this.getLastPacketTime()).toString());
        
        if (this.getTransmitter() != null) {
            retval.put(Symmitron.PACKETS_SENT, new Long(this.getTransmitter().getPacketsSent()).toString());
        } else {
            retval.put(Symmitron.PACKETS_SENT, "0");
        }
        retval.put(Symmitron.PACKETS_RECEIVED, new Long(this.getPacketsReceived()).toString());
        return retval;
        
    }
    
    void setVisited(long stamp) {
        visited = stamp; 
    }

    boolean isVisited(long stamp) {
        return this.visited == stamp;
    }
    
    void clearStrayPacketMap() {
        this.strayPacketCounters.clear();
    }

    public void recordStrayPacket(String hostAddress) {
        if (!strayPacketAlarmTable.contains(hostAddress)) {
            if (!this.strayPacketCounters.containsKey(hostAddress)) {
                this.strayPacketCounters.put(hostAddress, 0);
            }
            int current = this.strayPacketCounters.get(hostAddress).intValue();
            this.strayPacketCounters.put(hostAddress, current + 1);
            if (current + 1 == 10 * 1024) {
                try {
                    SymmitronServer.alarmClient.raiseAlarm(
                            SymmitronServer.SIPX_RELAY_STRAY_PACKET_ALARM_ID,
                            hostAddress);
                    strayPacketAlarmTable.add(hostAddress);
                    this.strayPacketCounters.remove(hostAddress);
                } catch (XmlRpcException e) {
                    logger.error("Problem sending Alarm", e);
                    return;
                }
            }
        }

    }
    
    

}
