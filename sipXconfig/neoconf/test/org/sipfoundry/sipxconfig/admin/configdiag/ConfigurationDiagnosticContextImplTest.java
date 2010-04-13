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

import java.io.File;
import java.util.List;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.admin.configdiag.ConfigurationDiagnosticResult.Status;

public class ConfigurationDiagnosticContextImplTest extends TestCase {

    private ConfigurationDiagnosticContextImpl m_out;

    public void setUp() {
        m_out = new ConfigurationDiagnosticContextImpl();
        String testFilename = getClass().getClassLoader().getResource("org/sipfoundry/sipxconfig/admin/configdiag/10simple.test.xml").getFile();
        String descriptorPath = new File(testFilename).getParent();
        m_out.setDescriptorPath(descriptorPath);
    }

    public void testGetConfigurationTests() throws Exception {
        List<ConfigurationDiagnostic> allTests = m_out.getConfigurationTests();
        assertEquals("Expected 2 diagnostic tests", 2, allTests.size());

        assertEquals("Wrong test name for first test", "simple", allTests.get(0).getName());
        assertEquals("Wrong test name for second test", "dhcp", allTests.get(1).getName());

        // test one config diag to assert it is properly configured
        ConfigurationDiagnostic simpleDiag = allTests.get(1);

        // need to override command location for test
        simpleDiag.setCommand(new ExternalCommand() {
            public int execute() {
                return 0;
            }
        });

        simpleDiag.call();
        assertEquals("Simple diag failed on execute.", Status.Success, simpleDiag.getResult().getStatus());
    }
}
