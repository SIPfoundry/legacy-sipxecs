/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.phone.ciscoplus;

import org.sipfoundry.sipxconfig.phone.Phone;

/**
 * Support for ciscoplus 79xx
 */
public abstract class Ciscoplus extends Phone {

    public static final String PORT = "port";

    public static final String SIP = "sip";

    protected Ciscoplus() {
    }

    @Override
    public CiscoplusModel getModel() {
        return (CiscoplusModel) super.getModel();
    }

    public int getMaxLineCount() {
        return getModel().getMaxLineCount();
    }

    @Override
    public void restart() {
        sendCheckSyncToFirstLine();
    }
}
