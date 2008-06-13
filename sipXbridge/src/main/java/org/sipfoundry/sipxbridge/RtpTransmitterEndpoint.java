package org.sipfoundry.sipxbridge;

import java.net.InetAddress;
import java.net.InetSocketAddress;

import javax.sdp.MediaDescription;
import javax.sdp.SessionDescription;

import org.apache.log4j.Logger;
import org.sipfoundry.sipxbridge.symmitron.SymTransmitterEndpoint;

public class RtpTransmitterEndpoint extends SymTransmitterEndpoint {
    
    private static Logger logger = Logger.getLogger(RtpTransmitterEndpoint.class);
    
    /*
     * Session description.
     */
    protected  SessionDescription sessionDescription;

   
    /**
     * Get the associated session description.
     * 
     * @return
     */
    SessionDescription getSessionDescription() {
        return sessionDescription;
    }
    
    public void setSessionDescription(SessionDescription sessionDescription, boolean isRtpSession) {
        if (this.sessionDescription != null) {
            logger.debug("WARNING -- replacing session description");

        }
        try {
            this.sessionDescription = sessionDescription;

            if (sessionDescription.getConnection() != null)
                this.ipAddress = sessionDescription.getConnection()
                        .getAddress();

            MediaDescription mediaDescription = (MediaDescription) sessionDescription
                    .getMediaDescriptions(true).get(0);

            if (mediaDescription.getConnection() != null) {

                ipAddress = mediaDescription.getConnection().getAddress();

            }

            if ( isRtpSession )
                this.port = mediaDescription.getMedia().getMediaPort();
            else
                this.port = mediaDescription.getMedia().getMediaPort() + 1;

            if (logger.isDebugEnabled()) {
                logger.debug("isTransmitter = true : Setting ipAddress : "
                        + ipAddress);
                logger.debug("isTransmitter = true : Setting port " + port);
            }

            InetAddress inetAddress = InetAddress.getByName(ipAddress);
            this.setSocketAddress(new InetSocketAddress(inetAddress, this.port));

            /*
             * We use the same datagram channel for sending and receiving.
             */
            this.datagramChannel = this.getSym().getReceiver()
                    .getDatagramChannel();
            if (this.datagramChannel == null) {
                logger.error("Setting datagram channel to NULL! ");
            }

            this.onHold = false;

            assert this.datagramChannel != null;

        } catch (Exception ex) {
            logger.error("Unexpected exception ", ex);
            throw new RuntimeException("Unexpected exception setting sdp", ex);
        }
    }

    
   
}
