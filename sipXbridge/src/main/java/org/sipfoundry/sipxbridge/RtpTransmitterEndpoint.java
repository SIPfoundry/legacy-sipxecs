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

	private boolean isOffer;

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
        this.symTransmitter.setIpAddressAndPort(ipAddress, port, keepaliveInterval,
                keepaliveMethod);
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

    public void setOnHold(boolean flag) {
        this.symTransmitter.setOnHold(flag);
    }

    void setSessionDescription(SessionDescription sessionDescription, boolean isOffer) {
        if (this.sessionDescription != null) {
            logger.debug("WARNING -- replacing session description");

        }
        
        this.isOffer = isOffer;
        try {
            this.sessionDescription = sessionDescription;
            this.ipAddress = null;

            this.keepAliveInterval = Gateway.getMediaKeepaliveMilisec();
            this.ipAddress = SipUtilities.getSessionDescriptionMediaIpAddress(sessionDescription);
            this.port = SipUtilities.getSessionDescriptionMediaPort(sessionDescription);

            if (logger.isDebugEnabled()) {
                logger.debug("isTransmitter = true : Setting ipAddress : " + ipAddress);
                logger.debug("isTransmitter = true : Setting port " + port);
            }

            this.symTransmitter.setIpAddressAndPort(ipAddress, port, keepAliveInterval,
                    keepAliveMethod);
            this.symTransmitter.setOnHold(false);

        } catch (Exception ex) {
            logger.error("Unexpected exception ", ex);
            throw new RuntimeException("Unexpected exception setting sdp", ex);
        }
    }




	boolean isOffer() {
		return isOffer;
	}



    

}
