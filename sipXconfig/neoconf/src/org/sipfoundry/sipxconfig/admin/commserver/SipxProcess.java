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

import org.springframework.beans.factory.annotation.Required;

public class SipxProcess {
    private Boolean m_enabled;
    private SipxProcessContext m_sipxProcessContext;
    private String m_processName;

    public boolean isEnabled() {
        if (m_enabled == null) {
            m_enabled = searchProcess();
        }
        return m_enabled;
    }

    private boolean searchProcess() {
        ServiceStatus [] servStatus = null;
        for (Location location : m_sipxProcessContext.getLocations()) {
            servStatus = m_sipxProcessContext.getStatus(location);
            for (ServiceStatus status : servStatus) {
                if (status.getServiceName().equals(m_processName)) {
                    return true;
                }
            }
        }
        return false;
    }

    @Required
    public void setSipxProcessContext(SipxProcessContext sipxProcessContext) {
        m_sipxProcessContext = sipxProcessContext;
    }

    @Required
    public void setProcessName(String processName) {
        m_processName = processName;
    }

    public String getProcessName() {
        return m_processName;
    }

}
