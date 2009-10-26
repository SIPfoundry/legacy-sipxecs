/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxrelay;


public class SymEndpointImpl implements SymEndpointInterface {
    private SymmitronClient symmitronClient;
    
    private String id;
    private String ipAddress;
    private int port;

   

    public SymEndpointImpl() {
        
    }
    
    protected SymEndpointImpl( SymmitronClient symmitronClient ) {
        this.symmitronClient = symmitronClient;
    }
    
    public String getId() {
       return id;
    }

    public String getIpAddress() {
      
        return ipAddress;
    }

    public int getPort() {
        
        return port;
    }

    /**
     * @param ipAddress the ipAddress to set
     */
    public void setIpAddress(String ipAddress) {
        this.ipAddress = ipAddress;
    }

    /**
     * @param port the port to set
     */
    public void setPort(int port) {
        this.port = port;
    }

    /**
     * @param id the id to set
     */
    public void setId(String id) {
        this.id = id;
    }

   

}
