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

import org.sipfoundry.sipxconfig.admin.configdiag.ConfigurationDiagnosticResult.Status;

import junit.framework.TestCase;

public class CompositeExecutionTaskTest extends TestCase {

    private CompositeExecutionTask m_out;
    private ConfigurationDiagnosticResultParser m_resultParser;

    public void setUp() {
        m_out = new CompositeExecutionTaskImpl();

        ConfigurationDiagnosticResult successResult = new ConfigurationDiagnosticResult();
        successResult.setExitStatus(0);
        successResult.setMessage("Success");
        successResult.setStatus(Status.Success);

        ConfigurationDiagnosticResult errorResult = new ConfigurationDiagnosticResult();
        errorResult.setExitStatus(1);
        errorResult.setMessage("Error");
        errorResult.setStatus(Status.Error);

        m_resultParser = new ConfigurationDiagnosticResultParser();
        m_resultParser.addResult(successResult);
        m_resultParser.addResult(errorResult);
    }

    public void testExecuteWithOneTask() {
        SimpleExecutionTask task = new SimpleExecutionTask();
        task.setCommand(new ExternalCommand() {
            public int execute() {
                return 0;
            }
        });
        task.setResultParser(m_resultParser);
        ConfigurationDiagnosticResult result = new ConfigurationDiagnosticResult();
        task.setResult(result);

        m_out.addExecutionTask(task);
        m_out.execute();

        assertEquals(Status.Success, result.getStatus());
    }

    public void testExecuteWithTwoTasks() {
        SimpleExecutionTask firstTask = new SimpleExecutionTask();
        firstTask.setCommand(new ExternalCommand() {
            public int execute() {
                return 0;
            }
        });
        firstTask.setResultParser(m_resultParser);
        ConfigurationDiagnosticResult firstResult = new ConfigurationDiagnosticResult();
        firstTask.setResult(firstResult);

        SimpleExecutionTask secondTask = new SimpleExecutionTask();
        secondTask.setCommand(new ExternalCommand() {
            public int execute() {
                return 1;
            }
        });
        secondTask.setResultParser(m_resultParser);
        ConfigurationDiagnosticResult secondResult = new ConfigurationDiagnosticResult();
        secondTask.setResult(secondResult);

        m_out.addExecutionTask(firstTask);
        m_out.addExecutionTask(secondTask);
        m_out.execute();

        assertEquals(Status.Success, firstResult.getStatus());
        assertEquals(Status.Error, secondResult.getStatus());
    }
}
