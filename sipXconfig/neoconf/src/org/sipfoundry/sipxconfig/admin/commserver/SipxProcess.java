/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.commserver;

import org.sipfoundry.sipxconfig.admin.commserver.ServiceStatus.Status;
import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessModel.ProcessName;
import org.springframework.beans.factory.annotation.Required;

public class SipxProcess {
    private Boolean m_enabled;
    private SipxProcessContext m_sipxProcessContext;
    private ProcessName m_processName;

    public SipxProcess(SipxProcessContext sipxProcessContext, ProcessName processName) {
        m_sipxProcessContext = sipxProcessContext;
        m_processName = processName;
    }

    public boolean isEnabled() {
        if (m_enabled == null) {
            m_enabled = searchProcess();
        }
        return m_enabled;
    }

    private boolean searchProcess() {
        boolean enabled = false;
        String name = m_processName.getName();
        ServiceStatus[] servStatus = null;
        for (Location location : m_sipxProcessContext.getLocations()) {
            servStatus = m_sipxProcessContext.getStatus(location);
            for (ServiceStatus status : servStatus) {
                if (status.getServiceName().equals(name)) {
                    enabled |= status.getStatus().equals(Status.STARTED);
                }
            }
        }
        return enabled;
    }

    @Required
    public void setSipxProcessContext(SipxProcessContext sipxProcessContext) {
        m_sipxProcessContext = sipxProcessContext;
    }

    @Required
    public void setProcessName(ProcessName processName) {
        m_processName = processName;
    }
}
