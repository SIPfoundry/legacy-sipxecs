/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxbridge;

import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.UnknownHostException;

import javax.sdp.MediaDescription;
import javax.sdp.SessionDescription;

import org.apache.log4j.Logger;
import org.sipfoundry.sipxbridge.symmitron.KeepaliveMethod;
import org.sipfoundry.sipxbridge.symmitron.SymTransmitterEndpointImpl;
import org.sipfoundry.sipxbridge.symmitron.SymTransmitterEndpointInterface;
import org.sipfoundry.sipxbridge.symmitron.SymmitronClient;

class RtpTransmitterEndpoint {

    private static Logger logger = Logger.getLogger(RtpTransmitterEndpoint.class);

    /*
     * Session description.
     */
    protected SessionDescription sessionDescription;

    private SymTransmitterEndpointImpl symTransmitter;

    private RtpSession rtpSession;
    
    private String ipAddress;
    
    private int port;

    private int keepAliveInterval;

    private KeepaliveMethod keepAliveMethod = KeepaliveMethod.NONE;

    public RtpTransmitterEndpoint(RtpSession rtpSession, SymmitronClient symmitronClient) {
        this.rtpSession = rtpSession;
        this.symTransmitter = symmitronClient.createSymTransmitter(
                this.rtpSession.getSym());

    }

    public String getIpAddress() {
        return ipAddress;
    }

    public int getPort() {
        return port;
    }
    
    public void setIpAddressAndPort(String ipAddress, int port, int keepAliveInterval, KeepaliveMethod keepAliveMethod) throws UnknownHostException {
        this.ipAddress = ipAddress;
        this.port = port;
        this.keepAliveInterval = keepAliveInterval;
        this.keepAliveMethod = keepAliveMethod;
        this.symTransmitter.setIpAddressAndPort(ipAddress, port, keepAliveInterval, keepAliveMethod);
        
    }

    public void setIpAddressAndPort(String ipAddress, int port) throws UnknownHostException {
        this.ipAddress = ipAddress;
        this.port = port;
        logger.debug("setIpAddressAndPort" + ipAddress + " port " + port);
        this.symTransmitter.setIpAddressAndPort(ipAddress, port,keepAliveInterval, keepAliveMethod);

    }
    
    public void setIpAddressAndPort(int keepaliveInterval, KeepaliveMethod keepaliveMethod) throws UnknownHostException {
        this.keepAliveInterval = keepaliveInterval;
        this.keepAliveMethod = keepaliveMethod;
        this.symTransmitter.setIpAddressAndPort(ipAddress, port,keepaliveInterval, keepaliveMethod);
    }

    public SymTransmitterEndpointInterface getSymTransmitter() {
        return this.symTransmitter;
    }

    /**
     * Get the associated session description.
     * 
     * @return
     */
    SessionDescription getSessionDescription() {
        return sessionDescription;
    }

    public void setOnHold(boolean flag) {
        this.symTransmitter.setOnHold(flag);
    }

    void setSessionDescription(SessionDescription sessionDescription) {
        if (this.sessionDescription != null) {
            logger.debug("WARNING -- replacing session description");

        }
        try {
            this.sessionDescription = sessionDescription;
            this.ipAddress = null;
            

            if (sessionDescription.getConnection() != null) {
                ipAddress = sessionDescription.getConnection().getAddress();
            }

            MediaDescription mediaDescription = (MediaDescription) sessionDescription
                    .getMediaDescriptions(true).get(0);

            if (mediaDescription.getConnection() != null) {

                ipAddress = mediaDescription.getConnection().getAddress();

            }

            this.port = mediaDescription.getMedia().getMediaPort();

            if (logger.isDebugEnabled()) {
                logger.debug("isTransmitter = true : Setting ipAddress : " + ipAddress);
                logger.debug("isTransmitter = true : Setting port " + port);
            }

           if ( keepAliveMethod != null ) {
               this.symTransmitter.setIpAddressAndPort(ipAddress, port,keepAliveInterval, keepAliveMethod);
               this.symTransmitter.setOnHold(false);
           }
           

        } catch (Exception ex) {
            logger.error("Unexpected exception ", ex);
            throw new RuntimeException("Unexpected exception setting sdp", ex);
        }
    }

}
