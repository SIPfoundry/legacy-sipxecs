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

public class ConfigurationDiagnosticResultParserTest extends TestCase {

    private ConfigurationDiagnosticResultParser m_out;

    public void setUp() {
        m_out = new ConfigurationDiagnosticResultParser();

        ConfigurationDiagnosticResult successResult = new ConfigurationDiagnosticResult();
        successResult.setExitStatus(0);
        successResult.setStatus(Status.Success);
        successResult.setMessage("Success");

        ConfigurationDiagnosticResult errorResult = new ConfigurationDiagnosticResult();
        errorResult.setExitStatus(-1);
        errorResult.setStatus(Status.Error);
        errorResult.setMessage("Error");

        m_out.addResult(successResult);
        m_out.addResult(errorResult);
    }

    public void testParseResultWithUnknownExitStatus() {
        ConfigurationDiagnosticResult result = m_out.parseResult(-99);
        assertEquals(Status.Warning, result.getStatus());
    }

    public void testParseResult() {
        ConfigurationDiagnosticResult successResult = m_out.parseResult(0);
        assertEquals(0, successResult.getExitStatus());
        assertEquals(Status.Success, successResult.getStatus());
        assertEquals("Success", successResult.getMessage());

        ConfigurationDiagnosticResult errorResult = m_out.parseResult(-1);
        assertEquals(-1, errorResult.getExitStatus());
        assertEquals(Status.Error, errorResult.getStatus());
        assertEquals("Error", errorResult.getMessage());
    }

    public void testParseResultWithPassedConfigurationResult() {
        ConfigurationDiagnosticResult result = new ConfigurationDiagnosticResult();
        m_out.parseResult(0, result);
        assertEquals(0, result.getExitStatus());
        assertEquals(Status.Success, result.getStatus());
        assertEquals("Success", result.getMessage());
    }
}
