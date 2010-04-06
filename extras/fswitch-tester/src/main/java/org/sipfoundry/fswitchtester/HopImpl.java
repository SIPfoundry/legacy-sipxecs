/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.fswitchtester;

import javax.sip.address.Hop;

public class HopImpl implements Hop {
    
    private String host;
    private int port;
    private String transport;

    public HopImpl( String host, int port, String transport) {
        this.host = host;
        this.port = port;
        this.transport = transport;
    }

    public String getHost() {
       return host;
    }

    public int getPort() {
     
        return  port;
    }

    public String getTransport() {
        return transport;
    }

}
