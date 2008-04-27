package org.sipfoundry.sipxbridge;

import java.io.IOException;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.SocketException;
import java.nio.channels.DatagramChannel;
import java.util.Vector;

import javax.sdp.Connection;
import javax.sdp.MediaDescription;
import javax.sdp.Origin;
import javax.sdp.SessionDescription;

import org.apache.log4j.Logger;

/**
 * Receiver Endpoint.
 * 
 * @author mranga
 * 
 */
public class SymReceiverEndpoint extends SymEndpoint {

    private static Logger logger = Logger.getLogger(SymReceiverEndpoint.class);

    public SymReceiverEndpoint(int port) throws IOException {
        super();

        // Our IP address is the address where we are listening.

        this.ipAddress = Gateway.getLocalAddress();
        
        // Our port where we listen for stuff.
        
        this.port = port;

        if ( logger.isDebugEnabled())
            logger.debug("Creating SymReceiverEndpoint " + ipAddress + " port " + port);

        // The datagram channel which is used to receive packets.
        this.datagramChannel = DatagramChannel.open();

        // Set up to be non blocking
        this.datagramChannel.configureBlocking(false);

        InetAddress inetAddress = Gateway.getLocalAddressByName();

        // Allocate the datagram channel on which we will listen.
        socketAddress = new InetSocketAddress(inetAddress, port);

        datagramChannel.socket().bind(socketAddress);

    }

    public void setSessionDescription(SessionDescription sessionDescription) {
        Connection connection = sessionDescription.getConnection();
        try {
            this.sessionDescription = sessionDescription;

            if (connection != null)
                connection.setAddress(this.ipAddress);

            Origin origin = sessionDescription.getOrigin();
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
