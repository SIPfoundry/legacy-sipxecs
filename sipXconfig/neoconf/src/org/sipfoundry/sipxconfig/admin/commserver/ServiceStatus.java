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

import org.sipfoundry.sipxconfig.common.PrimaryKeySource;

public class ServiceStatus implements PrimaryKeySource {

    public static enum Status {
        Undefined,
        Running,
        Disabled,
        ShutDown,
        ConfigurationMismatch,
        ResourceRequired,
        ConfigurationTestFailed,
        Testing,
        Starting,
        Stopping,
        ShuttingDown,
        Failed
    }

    private final String m_serviceName;
    private final Status m_status;

    public ServiceStatus(String serviceName, Status status) {
        m_serviceName = serviceName;
        m_status = status;
    }

    public String getServiceName() {
        return m_serviceName;
    }

    public Status getStatus() {
        return m_status;
    }

    public Object getPrimaryKey() {
        return m_serviceName;
    }
}
