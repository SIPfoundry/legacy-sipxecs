/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.service;

import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessModel.ProcessName;

public class SipxFreeswitchService extends SipxService {
    public static final String BEAN_ID = "sipxFreeswitchService";

    private static final ProcessName PROCESS_NAME = ProcessName.FREESWITCH_SERVER;

    @Override
    public ProcessName getProcessName() {
        return PROCESS_NAME;
    }
}
