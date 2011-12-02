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

import java.util.HashMap;
import java.util.Map;

public class ConfigurationDiagnosticResultParser {

    private Map<Integer, ConfigurationDiagnosticResult> m_ruleMap;

    public void addResult(ConfigurationDiagnosticResult result) {
        if (m_ruleMap == null) {
            m_ruleMap = new HashMap<Integer, ConfigurationDiagnosticResult>();
        }
        m_ruleMap.put(result.getExitStatus(), result);
    }

    private ConfigurationDiagnosticResult getResult(int exitStatus) {
        if (m_ruleMap == null) {
            return ConfigurationDiagnosticResult.createUnknown(exitStatus);
        }
        ConfigurationDiagnosticResult result = m_ruleMap.get(exitStatus);
        if (result == null) {
            return ConfigurationDiagnosticResult.createUnknown(exitStatus);
        }
        return result;
    }

    public ConfigurationDiagnosticResult parseResult(int exitStatus) {
        return getResult(exitStatus);
    }

    public void parseResult(int exitStatus, ConfigurationDiagnosticResult result) {
        ConfigurationDiagnosticResult localResult = getResult(exitStatus);
        result.setExitStatus(localResult.getExitStatus());
        result.setStatus(localResult.getStatus());
        result.setMessage(localResult.getMessage());
    }
}
