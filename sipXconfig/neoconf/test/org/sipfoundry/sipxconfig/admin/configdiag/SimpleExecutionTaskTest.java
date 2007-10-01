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

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.admin.configdiag.ConfigurationDiagnosticResult.Status;

public class SimpleExecutionTaskTest extends TestCase {

    private ConfigurationDiagnosticResultParser m_resultParser;
    
    public void setUp() {
        m_resultParser = new ConfigurationDiagnosticResultParser();
        
        ConfigurationDiagnosticResult successResult = new ConfigurationDiagnosticResult();
        successResult.setExitStatus(0);
        successResult.setMessage("Success");
        successResult.setStatus(Status.Success);
        
        ConfigurationDiagnosticResult errorResult = new ConfigurationDiagnosticResult();
        errorResult.setExitStatus(1);
        errorResult.setMessage("Error");
        errorResult.setStatus(Status.Error);
        
        m_resultParser.addResult(successResult);
        m_resultParser.addResult(errorResult);
    }
    
    
    public void testExecuteWithSuccess() {
        ExternalCommand successfulCommand = new ExternalCommand() {
            public int execute() {
                return 0;
            }
        };
        
        SimpleExecutionTask out = new SimpleExecutionTask();
        out.setCommand(successfulCommand);
        out.setResultParser(m_resultParser);
        ConfigurationDiagnosticResult result = new ConfigurationDiagnosticResult();
        out.setResult(result);
        
        out.execute();
        assertEquals(Status.Success, result.getStatus());
    }
    
    public void testExecuteWithError() {
        ExternalCommand failedCommand = new ExternalCommand() {
            public int execute() {
                return 1;
            }
        };
        
        SimpleExecutionTask out = new SimpleExecutionTask();
        out.setCommand(failedCommand);
        out.setResultParser(m_resultParser);
        ConfigurationDiagnosticResult result = new ConfigurationDiagnosticResult();
        out.setResult(result);
        
        out.execute();
        assertEquals(Status.Error, result.getStatus());
    }
}
