/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.admin.configdiag;

import java.io.Serializable;
import java.util.HashMap;
import java.util.Map;

import org.sipfoundry.sipxconfig.admin.configdiag.ConfigurationDiagnosticResult.Status;

public class ConfigurationDiagnosticResultParser implements Serializable {

    private Map<Integer, ConfigurationDiagnosticResult> m_ruleMap;

    public void addResult(ConfigurationDiagnosticResult result) {
        if (m_ruleMap == null) {
            m_ruleMap = new HashMap<Integer, ConfigurationDiagnosticResult>();
        }

        m_ruleMap.put(result.getExitStatus(), result);
    }

    public ConfigurationDiagnosticResult parseResult(int exitStatus) {
        if (m_ruleMap != null && m_ruleMap.containsKey(exitStatus)) {
            return m_ruleMap.get(exitStatus);
        }
        return ConfigurationDiagnosticResult.UNKNOWN_RESULT;
    }

    public void parseResult(int exitStatus, ConfigurationDiagnosticResult result) {
        if (m_ruleMap != null && m_ruleMap.containsKey(exitStatus)) {
            ConfigurationDiagnosticResult localresult = m_ruleMap.get(exitStatus);
            result.setExitStatus(localresult.getExitStatus());
            result.setStatus(localresult.getStatus());
            result.setMessage(localresult.getMessage());
        } else {
            result.setExitStatus(exitStatus);
            result.setStatus(Status.Unknown);
            result.setMessage("Unknown exit code: " + exitStatus);
        }
    }
}
