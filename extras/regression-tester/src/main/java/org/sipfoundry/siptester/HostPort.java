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
    private String host;
    private int port;
    HostPort(String host, int port ) {
        this.host = host;
        this.port = port;
    }
    
    /**
     * @return the host
     */
    public String getHost() {
        return host;
    }
    /**
     * @return the port
     */
    public int getPort() {
        return port;
    }
}