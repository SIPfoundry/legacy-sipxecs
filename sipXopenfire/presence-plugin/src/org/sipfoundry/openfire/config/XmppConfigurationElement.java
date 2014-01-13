/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.openfire.config;

import org.apache.log4j.Logger;

public abstract class XmppConfigurationElement
{
    private static Logger logger = Logger.getLogger(XmppConfigurationElement.class);

    private XmppAccountStatus status = XmppAccountStatus.NEW;

    public XmppAccountStatus getStatus()
    {
        return status;
    }

    public void setStatus( XmppAccountStatus status )
    {
        this.status = status;
    }

    public void update() throws Exception {
        logger.error("Dealing with unexpected class type " + getClass());
    }
}
