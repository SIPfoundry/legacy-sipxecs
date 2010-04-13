/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxrelay;

import java.io.IOException;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.nio.channels.DatagramChannel;

import org.apache.log4j.Logger;


/**
 * Receiver Endpoint.
 * 
 * @author mranga
 * 
 */
final class SymReceiverEndpoint extends SymEndpoint {

    private static Logger logger = Logger.getLogger(SymReceiverEndpoint.class.getPackage().getName());
    
    private  InetSocketAddress socketAddress;

    public SymReceiverEndpoint(int port) throws IOException {
        super();

        // Our IP address is the address where we are listening.

        setIpAddress(SymmitronServer.getLocalAddress());
        
        // Our port where we listen for stuff.
        
        setPort(port);

        if ( logger.isDebugEnabled()) {
            logger.debug("Creating SymReceiverEndpoint " + getIpAddress() + " port " + port);
        }

        // The datagram channel which is used to receive packets.
        this.datagramChannel = DatagramChannel.open();
        
        // Set up to be non blocking
        this.datagramChannel.configureBlocking(false);
        

        InetAddress inetAddress = super.getInetAddress();
        //SymmitronServer.getLocalAddressByName();

        // Allocate the datagram channel on which we will listen.
        socketAddress = new InetSocketAddress(inetAddress, port);

        datagramChannel.socket().bind(socketAddress);
        
        

    }

   

}
