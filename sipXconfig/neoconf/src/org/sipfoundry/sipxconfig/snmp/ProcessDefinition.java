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
    private String m_regexp;

    public ProcessDefinition(String process) {
        m_process = process;
    }

    public ProcessDefinition(String process, String regexp) {
        this(process);
        m_regexp = regexp;
    }

    public String getProcess() {
        return m_process;
    }

    public String getRegexp() {
        return m_regexp;
    }
}
