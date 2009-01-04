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

    private final String m_serviceBeanId;
    private final Status m_status;

    public ServiceStatus(String serviceBeanId, Status status) {
        m_serviceBeanId = serviceBeanId;
        m_status = status;
    }

    public String getServiceBeanId() {
        return m_serviceBeanId;
    }

    public Status getStatus() {
        return m_status;
    }

    public Object getPrimaryKey() {
        return m_serviceBeanId;
    }
}
