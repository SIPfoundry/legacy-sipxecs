/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.phone.cisco;

import org.sipfoundry.sipxconfig.phone.Phone;

/**
 * Support for Cisco 7940/7960
 */
public abstract class CiscoPhone extends Phone {

    public static final String PORT = "port";

    public static final String SIP = "sip";

    protected CiscoPhone() {
    }

    @Override
    public CiscoModel getModel() {
        return (CiscoModel) super.getModel();
    }

    public int getMaxLineCount() {
        return getModel().getMaxLineCount();
    }

    @Override
    public void restart() {
        sendCheckSyncToFirstLine();
    }
}
