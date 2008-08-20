/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxbridge;

import java.io.IOException;
import java.util.Random;
import java.util.Vector;

import javax.sdp.Connection;
import javax.sdp.MediaDescription;
import javax.sdp.Origin;
import javax.sdp.SdpFactory;
import javax.sdp.SessionDescription;

import org.apache.log4j.Logger;
import org.sipfoundry.sipxbridge.symmitron.SymEndpointInterface;


class RtpReceiverEndpoint implements SymEndpointInterface {

    private Origin origin;

    private SessionDescription sessionDescription;

    private static Logger logger = Logger.getLogger(RtpReceiverEndpoint.class);

    private static String originatorName = "sipxbridge";

    private long sessionId;

    private int sessionVersion;

    private SymEndpointInterface symReceiverEndpoint;

    private String ipAddress;
    
    private boolean sdpQueried;

    RtpReceiverEndpoint(SymEndpointInterface symReceiverEndpoint)  {
        this.symReceiverEndpoint = symReceiverEndpoint;
        this.ipAddress = symReceiverEndpoint.getIpAddress();
        try {
            this.sessionId = Math.abs(new Random().nextLong());
            this.sessionVersion = 1;

            String address = this.getIpAddress();
            origin = SdpFactory.getInstance().createOrigin(originatorName, sessionId,
                    sessionVersion, "IN", "IP4", address);
        } catch (Exception ex) {
            throw new RuntimeException("Unexpected exception creating origin ", ex);
        }
    }

    
    public String getIpAddress() {
        return this.ipAddress;
    }
    
    public int getPort() {
        return this.symReceiverEndpoint.getPort();
    }
    
    public void setIpAddress(String ipAddress) {
        this.ipAddress = ipAddress;
    }
   

    SessionDescription getSessionDescription() {
        return this.sessionDescription;
    }

    void setSessionDescription(SessionDescription sessionDescription) {
        if (this.sessionDescription != null && logger.isDebugEnabled()) {
            logger.debug("Old SD  = " + this.sessionDescription);
            logger.debug("newSD = " + sessionDescription);
        }
        Connection connection = sessionDescription.getConnection();

        try {
            /*
             * draft-ietf-sipping-sip-offeranswer-08 section 5.2.5 makes it clear that a UA cannot
             * change the session id field of the o-line when making a subsequent offer/answer.
             */
           

            this.sessionDescription = sessionDescription;
            this.sessionDescription.setOrigin(origin);

            if (connection != null) {
                connection.setAddress(this.getIpAddress());
            }

            Vector mds = sessionDescription.getMediaDescriptions(true);
            for (int i = 0; i < mds.size(); i++) {
                MediaDescription mediaDescription = (MediaDescription) mds.get(i);
                if (mediaDescription.getConnection() != null) {
                    mediaDescription.getConnection().setAddress(this.getIpAddress());
                }

                mediaDescription.getMedia().setMediaPort(this.getPort());
            }

            if (logger.isDebugEnabled()) {
                logger.debug("Setting ipAddress : " + this.getIpAddress());
                logger.debug("Setting port " + this.getPort());
            }
        } catch (Exception ex) {
            logger.error("Unexpected exception ", ex);
            throw new RuntimeException("Unexpected exception", ex);
        }
    }


    /**
     * @param sdpQueried the sdpQueried to set
     */
    void setSdpQueried(boolean sdpQueried) {
        this.sdpQueried = sdpQueried;
    }


    /**
     * @return the sdpQueried
     */
    boolean isSdpQueried() {
        return sdpQueried;
    }


    
}
