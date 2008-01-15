/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 *
 */
package org.sipfoundry.sipxconfig.admin.configdiag;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.admin.configdiag.ConfigurationDiagnosticResult.Status;

public class ConfigurationDiagnosticResultTest extends TestCase {

    public void testIsFailing() {
        ConfigurationDiagnosticResult result = new ConfigurationDiagnosticResult();
        result.setStatus(Status.Error);
        assertTrue(result.isFailing());
        result.setStatus(Status.Warning);
        assertTrue(result.isFailing());
        result.setStatus(Status.Fatal);
        assertTrue(result.isFailing());
        result.setStatus(Status.InProgress);
        assertFalse(result.isFailing());
        result.setStatus(Status.Unknown);
        assertFalse(result.isFailing());
        result.setStatus(Status.Success);
        assertFalse(result.isFailing());
    }
}
