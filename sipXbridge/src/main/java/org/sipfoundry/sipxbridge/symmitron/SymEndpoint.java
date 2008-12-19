/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxbridge.symmitron;

import java.net.UnknownHostException;
import java.nio.channels.DatagramChannel;
import java.util.HashMap;
import java.util.Map;
import java.util.Random;

/**
 * A media endpoint is an ip addres, port pair.
 * 
 * @author M. Ranganathan
 * 
 */
abstract class SymEndpoint implements SymEndpointInterface {

    private String id;

    /*
     * IP address
     */
    protected String ipAddress;

    /*
     * My port
     */
    protected int port;

    /*
     * Our datagram channel.
     */
    protected  DatagramChannel datagramChannel;

    private Sym sym;
 
    public Map<String,Object> toMap() {
       
            Map<String,Object> retval = new HashMap<String,Object>();
            retval.put("id", this.getId());
            retval.put("ipAddress", ipAddress);
            retval.put("port", new Integer(port) );
            return retval;
        

    }
    
    public void setIpAddressAndPort(String ipAddress, int port) throws UnknownHostException {
        this.ipAddress = ipAddress;
        this.port = port;
    }

    /**
     * Constructor sym endpoint.
     * 
     * @param isTransmitter -- whether or not this is a transmitter.
     * @param rtpPortLowerBound -- where to start counting for port allocation.
     * 
     */
    protected SymEndpoint()  {

        this.id = "sym-endpoint:" + Math.abs(new Random().nextLong());
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
     * @return the sym session.
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
    public void setPort(int port) throws IllegalArgumentException {
        if ( port < 0 ) throw new IllegalArgumentException("Bad port "+ port);
        this.port = port;
    }
    
    
    /**
     * Debugging routine for clients.
     * 
     * @return
     */
    public String getDatagramChannelState() {
        String state = "";
        if (this.datagramChannel.isOpen() ) {
            state += "OPEN";
        } else {
            return "CLOSED";
        } 
        
        if ( this.datagramChannel.isRegistered() ) {
             state += " REGISTERED";
        } else {
            return state + " NOT_REGISTERED";
        }
        
        if ( this.datagramChannel.isConnected()) {
            state += " CONNECTED";
        } else {
            state += " NOT_CONNECTED";
        }
        
        state += "host = " + this.datagramChannel.socket().getInetAddress() + "; local port = " + this.datagramChannel.socket().getLocalPort();
        
        
        return state;
        
       
    }

}
