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

public class SipxBridgeService extends SipxService {
    private static final String BEAN_ID = "sipxBridgeService";
    private static final ProcessName PROCESS_NAME = ProcessName.SBC_BRIDGE;
    
    @Override
    public ProcessName getProcessName() {
        return PROCESS_NAME;
    }

    @Override
    public String getBeanId() {
        return BEAN_ID;
    }
}
