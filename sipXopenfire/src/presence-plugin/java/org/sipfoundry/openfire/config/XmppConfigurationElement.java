package org.sipfoundry.openfire.config;

public abstract class XmppConfigurationElement
{
    private XmppAccountStatus status = XmppAccountStatus.NEW;

    public XmppAccountStatus getStatus()
    {
        return status;
    }
    
    public void setStatus( XmppAccountStatus status )
    {
        this.status = status;
    }
}
