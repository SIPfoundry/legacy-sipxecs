/*
 * 
 * 
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */

package org.sipfoundry.siptester;

class HostPort {
    protected String ipAddress;
    protected int port;
    HostPort(String host, int port ) {
        this.ipAddress = host;
        this.port = port;
    }
    
    HostPort() {
        
    }
    
    /**
     * @return the host
     */
    public String getIpAddress() {
        return ipAddress;
    }
    /**
     * @return the port
     */
    public int getPort() {
        return port;
    }
    
    @Override
    public int hashCode() {
        return ( ipAddress + ":" + port ).hashCode();
    }
    
    public String toString() {
        return ipAddress + ":" + port;
    }
    
    @Override
    public boolean equals(Object that) {
        HostPort other = (HostPort) that;
        return this.ipAddress.equals(other.ipAddress) && this.port == other.port;
    }
   
}