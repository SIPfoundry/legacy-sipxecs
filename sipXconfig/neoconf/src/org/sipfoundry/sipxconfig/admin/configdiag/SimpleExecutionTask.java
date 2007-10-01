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

public class SimpleExecutionTask implements ExecutionTask {
    private ConfigurationDiagnosticResultParser m_resultParser;
    private ExternalCommand m_command;
    private ConfigurationDiagnosticResult m_result;

    public void execute() {
        m_resultParser.parseResult(m_command.execute(), m_result);
    }

    public void setResultParser(ConfigurationDiagnosticResultParser resultParser) {
        m_resultParser = resultParser;
    }

    public void setCommand(ExternalCommand command) {
        m_command = command;
    }

    public void setResult(ConfigurationDiagnosticResult result) {
        m_result = result;
    }
}
