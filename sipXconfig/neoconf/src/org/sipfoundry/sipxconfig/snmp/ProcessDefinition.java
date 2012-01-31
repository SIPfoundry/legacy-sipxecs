/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.snmp;

public class ProcessDefinition {
    private String m_process;

    public ProcessDefinition(String process) {
        m_process = process;
    }

    public String getProcess() {
        return m_process;
    }
}
