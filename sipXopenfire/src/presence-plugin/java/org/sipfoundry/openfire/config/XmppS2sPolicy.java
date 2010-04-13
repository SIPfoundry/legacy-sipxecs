/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.openfire.config;

public class XmppS2sPolicy {
    private String xmppDomainName = "";
    private int xmppServerPort = 0;
    
    public XmppS2sPolicy () {
    }

    public String getXmppDomainName() {
        return xmppDomainName;
    }
    
    public void setXmppDomainName(String xmppDomainName) {
        this.xmppDomainName = xmppDomainName;
    }

    public int getXmppServerPort() {
        return xmppServerPort;
    }
    
    public void setXmppServerPort(int xmppServerPort) {
    	this.xmppServerPort = xmppServerPort;
    }
}
