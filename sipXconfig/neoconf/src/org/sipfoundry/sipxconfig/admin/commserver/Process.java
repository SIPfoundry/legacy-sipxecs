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

import org.apache.commons.lang.builder.HashCodeBuilder;
import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessModel.ProcessName;

public class Process extends Object {

    private String m_name;

    public Process() {
    }

    public Process(String name) {
        m_name = name;
    }

    public Process(ProcessName name) {
        m_name = name.getName();
    }

    public String getName() {
        return m_name;
    }

    public void setName(String name) {
        m_name = name;
    }

    public int hashCode() {
        return new HashCodeBuilder().append(m_name).hashCode();
    }

    public boolean equals(Object other) {
        if (!(other instanceof Process)) {
            return false;
        }
        if (this == other) {
            return true;
        }
        Process proc = (Process) other;
        if (m_name == null) {
            return (proc.getName() == null);
        }
        return m_name.equals(proc.getName());
    }
}
