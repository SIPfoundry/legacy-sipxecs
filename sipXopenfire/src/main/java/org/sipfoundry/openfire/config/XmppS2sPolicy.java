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
