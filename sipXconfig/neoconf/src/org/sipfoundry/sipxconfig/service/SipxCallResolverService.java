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

public class SipxCallResolverService extends SipxService {
    public static final String BEAN_ID = "sipxCallResolverService";

    private static final ProcessName PROCESS_NAME = ProcessName.CALL_RESOLVER;
    private int m_agentPort;
    
    @Override
    public ProcessName getProcessName() {
        return PROCESS_NAME;
    }

    public int getAgentPort() {
        return m_agentPort;
    }

    public void setAgentPort(int agentPort) {
        m_agentPort = agentPort;
    }
}
