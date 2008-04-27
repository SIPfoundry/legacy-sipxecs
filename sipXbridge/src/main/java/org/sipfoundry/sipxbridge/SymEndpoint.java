/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxbridge;

import java.io.IOException;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.SocketException;
import java.nio.ByteBuffer;
import java.nio.channels.ClosedChannelException;
import java.nio.channels.DatagramChannel;
import java.util.HashMap;
import java.util.Map;
import java.util.Random;
import java.util.TimerTask;
import java.util.Vector;

import javax.sdp.Connection;
import javax.sdp.MediaDescription;
import javax.sdp.Origin;
import javax.sdp.SessionDescription;

import org.apache.commons.beanutils.PropertyUtils;
import org.apache.log4j.Logger;

/**
 * A media endpoint is an ip addres, port pair.
 * 
 * @author M. Ranganathan
 * 
 */
public abstract class SymEndpoint implements SymEndpointInterface {

    private static Logger logger = Logger.getLogger(SymEndpoint.class);

    private String id;

    /*
     * IP address
     */
    protected String ipAddress;

    /*
     * My port
     */
    protected int port;

    // private InetAddress inetAddress;

    protected  InetSocketAddress socketAddress;


    /*
     * Session description.
     */
    protected  SessionDescription sessionDescription;

   
    /*
     * Our datagram channel.
     */
    protected  DatagramChannel datagramChannel;

    private Sym sym;
  
  

 
   

    public Map toMap() {
       
            Map<String,Object> retval = new HashMap<String,Object>();
            retval.put("id", this.getId());
            retval.put("ipAddress", ipAddress);
            retval.put("port", new Integer(port) );
            return retval;
        

    }

    /**
     * Constructor sym endpoint.
     * 
     * @param isTransmitter -- whether or not this is a transmitter.
     * @param rtpPortLowerBound -- where to start counting for port allocation.
     * 
     */
    protected SymEndpoint()  {

        this.id = "rtp-endpoint:" + Math.abs(new Random().nextLong());
    }

    /**
     * Get the unique ID for this endpoint.
     */
    public String getId() {
        return this.id;
    }

   

    /**
     * Get our RTP datagram channel.
     */
    public DatagramChannel getDatagramChannel() {
        return datagramChannel;
    }

    

   
  
    /**
     * Get the IP address associated with the endpoint ( transmitter will have the remote Ip address here).
     */

    public String getIpAddress() {
        return ipAddress;
    }

    /**
     * Get the port associated with the endpoint ( transmitter will have the remote port here ).
     * 
     */
    public int getPort() {
        return port;
    }

    

    /**
     * @param sym
     *            the rtpSession to set
     */
    public void setSym(Sym sym) {
        this.sym = sym;
    }

    /**
     * @return the rtpSession
     */
    public Sym getSym() {
        return sym;
    }

    /**
     * Set the ip address - the IP address will be either the local listen 
     * address or the remote address ( for transmitter).
     * 
     * @param ipAddress
     */
    public void setIpAddress(String ipAddress) {
        this.ipAddress = ipAddress;
    }

    
    /**
     * Set the port - the port will be either the local listen port or the remote port
     * (for the transmitter ).
     * 
     * @param port
     */
    public void setPort(int port) {
        this.port = port;
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
     * Set our session description parameter. Note that for a reciever, we fix
     * up the session description to be the IP address and port that we are
     * listening at.Make sure you call this method only with a cloned session
     * description because the fields of the sessiondescription are rewritten.
     * 
     * @param sessionDescription
     */
    abstract void setSessionDescription(SessionDescription sessionDescription);


 

}
