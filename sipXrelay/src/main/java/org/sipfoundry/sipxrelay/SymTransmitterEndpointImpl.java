/*
 * 
 * 
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 *
 */

package org.sipfoundry.sipxrelay;

import java.net.UnknownHostException;

import org.apache.log4j.Logger;

/**
 * The client side interface for the SymTransmitter.
 * 
 */
public class SymTransmitterEndpointImpl implements SymTransmitterEndpointInterface {

    private static Logger logger = Logger.getLogger("org.sipfoundry.sipxbridge");

    private SymmitronClient symmitronClient;
    private SymImpl sym;
    private String ipAddress;
    private int port;

    protected SymTransmitterEndpointImpl(SymmitronClient symmitronClient, SymImpl sym) {
        this.symmitronClient = symmitronClient;
        this.sym = sym;
    }

    public void setIpAddressAndPort(String ipAddress, int destinationPort, int keepAliveInterval,
            KeepaliveMethod keepAliveMethod) throws IllegalStateException, UnknownHostException {

        if (this.ipAddress != null && ipAddress.equals(this.ipAddress)
                && destinationPort == this.port) {
            /*
             * Note - this case can happen when there is a provisional response with SDP followed
             * by a 200 response with the same sdp.
             */
            logger
                    .debug("setIpAddressAndPort: resetting to previously set values -- returning silently");
            return;
        }
        if (logger.isDebugEnabled()) {
            logger.debug("setIpAddressAndPort " + sym.id + " ipAddress " + ipAddress + " port "
                    + destinationPort + " keepAliveMethod = " + keepAliveMethod);
        }

        this.ipAddress = ipAddress;
        this.port = destinationPort;
        this.symmitronClient.setRemoteEndpoint(sym, ipAddress, destinationPort,
                keepAliveInterval, keepAliveMethod);
    }

    public void setOnHold(boolean flag) {
        symmitronClient.setOnHold(sym.getId(), flag);
    }

    public String getIpAddress() {
        return ipAddress;
    }

    public int getPort() {
        return port;
    }

}
