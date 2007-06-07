/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.phone.linksys;

import org.sipfoundry.sipxconfig.phone.Phone;

/**
 * Support for linksys 941/942
 */
public abstract class Linksys extends Phone {

    public static final String PORT = "port";

    public static final String SIP = "sip";

    protected Linksys() {
    }

    @Override
    public LinksysModel getModel() {
        return (LinksysModel) super.getModel();
    }

    public int getMaxLineCount() {
        return getModel().getMaxLineCount();
    }

    @Override
    public void restart() {
        sendCheckSyncToFirstLine();
    }
}
