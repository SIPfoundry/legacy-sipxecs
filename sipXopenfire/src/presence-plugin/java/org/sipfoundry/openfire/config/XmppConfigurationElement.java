/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
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
