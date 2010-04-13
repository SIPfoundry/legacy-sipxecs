/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxrelay;

import java.net.InetAddress;
import java.net.UnknownHostException;
import java.nio.channels.DatagramChannel;
import java.util.HashMap;
import java.util.Map;
import java.util.Random;

import org.apache.log4j.Logger;

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
    private String ipAddress;

    /*
     * My port
     */
    private int port;

    /*
     * My Inet address.
     */
    private InetAddress inetAddress;

    /*
     * Our datagram channel.
     */
    protected  DatagramChannel datagramChannel;

    private Sym sym;

    private static Logger logger = Logger.getLogger(SymEndpoint.class);

    public Map<String,Object> toMap() {

            Map<String,Object> retval = new HashMap<String,Object>();
            retval.put("id", this.getId());
            retval.put("ipAddress", ipAddress);
            retval.put("port", new Integer(getPort()) );
            return retval;


    }

    public void setIpAddressAndPort(String ipAddress, int port) throws UnknownHostException {
        this.ipAddress = ipAddress;
        this.inetAddress = InetAddress.getByName(ipAddress);
        this.setPort(port);
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
     * @throws UnknownHostException
     */
    public void setIpAddress(String ipAddress) throws UnknownHostException {
    	if ( logger.isTraceEnabled() ) {
    		logger.trace("SymEndpoint: setIpAddress: " + ipAddress);
    	}
        this.ipAddress = ipAddress;
        if ( ipAddress != null ) {
            this.inetAddress = InetAddress.getByName(ipAddress);
        } else {
            this.inetAddress = null;
        }
    }


    /**
     * Set the port - the port will be either the local listen port or the remote port
     * (for the transmitter ).
     *
     * @param port
     */
    public void setPort(int port) throws IllegalArgumentException {
    	if ( logger.isTraceEnabled()) {
    		logger.trace("SymEndpoint : setPort : " + port);
    	}
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


    /**
     * @return the inetAddress
     */
    InetAddress getInetAddress() {
        return inetAddress;
    }

}
