/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxbridge;

import java.io.PrintWriter;
import java.io.StringWriter;
import java.net.UnknownHostException;

import javax.sdp.SessionDescription;

import org.apache.log4j.Logger;
import org.sipfoundry.sipxrelay.KeepaliveMethod;
import org.sipfoundry.sipxrelay.SymTransmitterEndpointImpl;
import org.sipfoundry.sipxrelay.SymTransmitterEndpointInterface;
import org.sipfoundry.sipxrelay.SymmitronClient;

class RtpTransmitterEndpoint {

    private static Logger logger = Logger.getLogger(RtpTransmitterEndpoint.class);

    /*
     * Session description.
     */
    private SessionDescription sessionDescription;

    private SymTransmitterEndpointImpl symTransmitter;

    private RtpSession rtpSession;

    private String ipAddress;

    private int port;

    private int keepAliveInterval;

    private KeepaliveMethod keepAliveMethod = KeepaliveMethod.NONE;

    private boolean isOffer;

    private boolean isOnHold;

    public RtpTransmitterEndpoint(RtpSession rtpSession, SymmitronClient symmitronClient) {
        this.rtpSession = rtpSession;
        this.symTransmitter = symmitronClient.createSymTransmitter(this.rtpSession.getSym());

    }

    public String getIpAddress() {
        return ipAddress;
    }

    public int getPort() {
        return port;
    }

    public void setIpAddressAndPort(String ipAddress, int port, int keepAliveInterval,
            KeepaliveMethod keepAliveMethod) throws UnknownHostException {
        logger.debug("setIpAddressAndPort " + this.rtpSession.getSym().getId() + " current  = "
                + this.ipAddress + "/" + this.port + " new " + ipAddress + "/" + port);

        this.ipAddress = ipAddress;
        this.port = port;
        this.keepAliveInterval = keepAliveInterval;
        this.keepAliveMethod = keepAliveMethod;
        this.symTransmitter.setIpAddressAndPort(ipAddress, port, keepAliveInterval,
                keepAliveMethod);

    }

    public void setIpAddressAndPort(String ipAddress, int port) throws UnknownHostException {
        logger.debug("setIpAddressAndPort " + this.rtpSession.getSym().getId() + " current  = "
                + this.ipAddress + "/" + this.port + " new " + ipAddress + "/" + port);

        this.ipAddress = ipAddress;
        this.port = port;

        this.symTransmitter.setIpAddressAndPort(ipAddress, port, keepAliveInterval,
                keepAliveMethod);

    }

    public void setIpAddressAndPort(int keepaliveInterval, KeepaliveMethod keepaliveMethod)
            throws UnknownHostException {
        logger.debug("setIpAddressAndPort " + this.rtpSession.getSym().getId() + " current  = "
                + this.ipAddress + "/" + this.port + " new " + ipAddress + "/" + port);

        this.keepAliveInterval = keepaliveInterval;
        this.keepAliveMethod = keepaliveMethod;
        if ( ipAddress != null ) {
            this.symTransmitter.setIpAddressAndPort(ipAddress, port, keepaliveInterval,
                keepaliveMethod);
        }
    }

    public void setKeepAliveMethod(KeepaliveMethod keepAliveMethod) {
        this.keepAliveMethod = keepAliveMethod;
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

    /**
     * Set or remove hold.
     */
    void setOnHold(boolean flag) {
        if (this.isOnHold == flag) {
            return;
        }
        
        if ( logger.isDebugEnabled() ) {
        	logger.debug("setOnHold : " + this.rtpSession.getSym().getId() + this + " : " + flag );
        }
        
        this.symTransmitter.setOnHold(flag);
        this.isOnHold = flag;
    }

    void setSessionDescription(SessionDescription sessionDescription, boolean isOffer) {
        if (this.sessionDescription != null) {
            logger.debug("replacing session description");
        } else {
        	logger.debug("setting session description");
        }

        

        this.isOffer = isOffer;
        try {

            this.keepAliveInterval = Gateway.getMediaKeepaliveMilisec();
            String ipAddress = SipUtilities
                    .getSessionDescriptionMediaIpAddress(sessionDescription);
            int port = SipUtilities.getSessionDescriptionMediaPort(sessionDescription);
                    

            if (logger.isDebugEnabled()) {
                logger.debug("isTransmitter = true : Setting ipAddress : " + ipAddress
                        + " Setting port " + port +  " isOffer = " + isOffer);
            }

            if (this.sessionDescription == null || !this.ipAddress.equals(ipAddress)
                    || this.port != port  ) {
                this.symTransmitter.setIpAddressAndPort(ipAddress, port, keepAliveInterval,
                        keepAliveMethod);
                this.ipAddress = ipAddress;
                this.port = port; 
                this.symTransmitter.setOnHold(false);
            }
            this.sessionDescription = sessionDescription;
            
          //  this.symTransmitter.setOnHold(this.rtpSession.isHoldRequest(sessionDescription));

        } catch (Exception ex) {
            logger.error("Unexpected exception ", ex);
            throw new SipXbridgeException("Unexpected exception setting sdp", ex);
        }
    }

    boolean isOffer() {
        return isOffer;
    }

    public boolean isOnHold() {
        return this.isOnHold;
    }

}
