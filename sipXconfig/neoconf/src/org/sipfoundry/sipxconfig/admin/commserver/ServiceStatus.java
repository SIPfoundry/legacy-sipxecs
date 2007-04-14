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

import java.util.List;

import org.apache.commons.lang.enums.Enum;
import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessContext.Process;
import org.sipfoundry.sipxconfig.common.PrimaryKeySource;

public class ServiceStatus implements PrimaryKeySource {

    public static final class Status extends Enum {
        public static final Status STARTING = new Status("Starting");
        public static final Status STARTED = new Status("Started");
        public static final Status STOPPED = new Status("Stopped");
        public static final Status FAILED = new Status("Failed");
        public static final Status UNKNOWN = new Status("Unknown");

        private Status(String name) {
            super(name);
        }

        public static List getAll() {
            return getEnumList(Status.class);
        }

        public static Status getEnum(String statusName) {
            return (Status) getEnum(Status.class, statusName);
        }
    }

    private Process m_process;
    private Status m_status;

    public ServiceStatus(Process process, Status status) {
        m_process = process;
        m_status = status;
    }

    public String getServiceName() {
        return m_process.getName();
    }

    public Status getStatus() {
        return m_status;
    }

    public Object getPrimaryKey() {
        return m_process;
    }
}
