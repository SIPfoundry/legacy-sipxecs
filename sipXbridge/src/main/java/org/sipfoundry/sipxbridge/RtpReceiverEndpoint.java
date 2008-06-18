package org.sipfoundry.sipxbridge;

import java.io.IOException;
import java.util.Vector;

import javax.sdp.Connection;
import javax.sdp.MediaDescription;
import javax.sdp.Origin;
import javax.sdp.SessionDescription;

import org.apache.log4j.Logger;
import org.sipfoundry.sipxbridge.symmitron.SymReceiverEndpoint;

public class RtpReceiverEndpoint extends SymReceiverEndpoint {
    
    private SessionDescription sessionDescription;
    
    private static Logger logger = Logger.getLogger(RtpReceiverEndpoint.class);
    
    public RtpReceiverEndpoint( int port ) throws IOException {
        super( port );
    }
    
    public SessionDescription getSessionDescription() {
        return this.sessionDescription;
    }
    
    public void setSessionDescription(SessionDescription sessionDescription, boolean isRtp) {
        if ( this.sessionDescription != null && logger.isDebugEnabled() ) {
            logger.debug("Old SD  = " + this.sessionDescription);
            logger.debug("newSD = " + sessionDescription);
        }
        Connection connection = sessionDescription.getConnection();
       
        try {
            Origin origin = sessionDescription.getOrigin();
            if ( this.sessionDescription != null ) {
                origin = this.sessionDescription.getOrigin();
                SipUtilities.incrementSdpVersion(sessionDescription);
            }
            this.sessionDescription = sessionDescription;
            
            sessionDescription.setOrigin(origin);

            if (connection != null)
                connection.setAddress(this.ipAddress);

            origin.setAddress(this.ipAddress);
            Vector mds = sessionDescription.getMediaDescriptions(true);
            for (int i = 0; i < mds.size(); i++) {
                MediaDescription mediaDescription = (MediaDescription) mds
                        .get(i);
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
