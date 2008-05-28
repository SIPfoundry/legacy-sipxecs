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

import org.sipfoundry.sipxconfig.common.BeanWithId;

public class Location extends BeanWithId {
    private String m_name;
    private String m_processMonitorUrl;
    private String m_replicationUrl;
    private String m_sipDomain;

    public String getName() {
        return m_name;
    }

    public void setName(String id) {
        m_name = id;
    }

    public String getProcessMonitorUrl() {
        return m_processMonitorUrl;
    }

    public void setProcessMonitorUrl(String processMonitorUrl) {
        m_processMonitorUrl = processMonitorUrl;
    }

    public String getReplicationUrl() {
        return m_replicationUrl;
    }

    public void setReplicationUrl(String replicationUrl) {
        m_replicationUrl = replicationUrl;
    }

    public String getSipDomain() {
        return m_sipDomain;
    }

    public void setSipDomain(String sipDomain) {
        m_sipDomain = sipDomain;
    }
}
