/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.admin.commserver;

import java.io.File;

import org.sipfoundry.sipxconfig.IntegrationTestCase;
import org.sipfoundry.sipxconfig.test.TestUtil;

public class InitialConfigTestIntegration extends IntegrationTestCase {
    private InitialConfig m_initialConfig;

    public void testGetArchiveStream() throws Exception {
        if (TestUtil.isWindows()) {
            // tries to run a shell script. ignore test
            return;
        }
        m_initialConfig.setBinDirectory(TestUtil.getTestSourceDirectory(this.getClass()));
        // create initial-config directory
        File file = new File(m_initialConfig.getTmpDirectory() + "/initial-config");
        file.mkdirs();
        m_initialConfig.getArchiveStream("test_location");

        // archive is created
        assertTrue(new File(m_initialConfig.getTmpDirectory() + "/initial-config/test_location.tar.gz").exists());
        m_initialConfig.deleteInitialConfigDirectory();
        // archive is deleted
        assertFalse(new File(m_initialConfig.getTmpDirectory() + "/initial-config/test_location.tar.gz").exists());
    }

    public void setInitialConfig(InitialConfig initialConfig) {
        m_initialConfig = initialConfig;
    }

}
