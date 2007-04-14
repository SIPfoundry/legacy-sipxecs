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

public class Location {
    private String m_id;
    private String m_processMonitorUrl;
    private String m_replicationUrl;
    private String m_sipDomain;

    public String getId() {
        return m_id;
    }

    public void setId(String id) {
        m_id = id;
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
