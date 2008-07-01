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
import org.sipfoundry.sipxbridge.symmitron.SymReceiverEndpoint;

public class RtpReceiverEndpoint extends SymReceiverEndpoint {

    private Origin origin;

    private SessionDescription sessionDescription;

    private static Logger logger = Logger.getLogger(RtpReceiverEndpoint.class);

    private static String originatorName = "sipxbridge";

    private long sessionId;

    private int sessionVersion;

    public RtpReceiverEndpoint(int port) throws IOException {
        super(port);
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

    @Override
    public void setIpAddress(String ipAddress) {

        super.setIpAddress(ipAddress);
        try {
            String address = this.getIpAddress();
            origin = SdpFactory.getInstance().createOrigin(originatorName, sessionId,
                    sessionVersion, "IN", "IP4", address);
        } catch (Exception ex) {
            throw new RuntimeException("Unexpected exception creating origin ", ex);
        }
    }

    public SessionDescription getSessionDescription() {
        return this.sessionDescription;
    }

    public void setSessionDescription(SessionDescription sessionDescription, boolean isRtp) {
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
                connection.setAddress(this.ipAddress);
            }

            Vector mds = sessionDescription.getMediaDescriptions(true);
            for (int i = 0; i < mds.size(); i++) {
                MediaDescription mediaDescription = (MediaDescription) mds.get(i);
                if (mediaDescription.getConnection() != null) {
                    mediaDescription.getConnection().setAddress(this.ipAddress);
                }

                mediaDescription.getMedia().setMediaPort(port);
            }

            if (logger.isDebugEnabled()) {
                logger.debug("Setting ipAddress : " + ipAddress);
                logger.debug("Setting port " + port);
            }
        } catch (Exception ex) {
            logger.error("Unexpected exception ", ex);
            throw new RuntimeException("Unexpected exception", ex);
        }
    }
}
