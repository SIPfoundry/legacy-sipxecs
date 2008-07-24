/*
 * 
 * 
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 *
 */

package org.sipfoundry.sipxbridge.symmitron;

import java.net.UnknownHostException;

/**
 * 
 */
public class SymTransmitterEndpointImpl implements SymTransmitterEndpointInterface {
    
    private SymmitronClient symmitronClient;
    private SymImpl sym;
    private String ipAddress;
    private int port;

    protected SymTransmitterEndpointImpl(SymmitronClient symmitronClient, SymImpl sym) {
        this.symmitronClient = symmitronClient;
        this.sym = sym;
    }

   
    public void setIpAddressAndPort(String ipAddress, int destinationPort, int keepAliveInterval, KeepaliveMethod keepAliveMethod) throws UnknownHostException {
       this.ipAddress = ipAddress;
       this.port = destinationPort;
       this.symmitronClient.setRemoteEndpoint(sym, ipAddress, destinationPort, keepAliveInterval, keepAliveMethod);
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
