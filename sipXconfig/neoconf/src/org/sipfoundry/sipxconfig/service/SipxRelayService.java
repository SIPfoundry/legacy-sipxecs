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

public class SipxRelayService extends SipxService {

    private static final ProcessName PROCESS_NAME = ProcessName.RELAY;
    
    @Override
    public ProcessName getProcessName() {
        return PROCESS_NAME;
    }

}
