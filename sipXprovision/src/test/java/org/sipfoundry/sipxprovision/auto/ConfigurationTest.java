/*
 *
 *
 * Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxprovision.auto;

import junit.framework.TestCase;

import java.io.ByteArrayOutputStream;
import java.io.PrintWriter;
import java.util.Properties;

/**
 * Tests for the Configuration class.
 *
 * @see Configuration
 *
 * @author Paul Mossman
 */
public class ConfigurationTest extends TestCase {

    private void useDefaultConfigFileDirectory() {

        System.setProperty("conf.dir", System.getProperty("basedir") + "/test/java/org/sipfoundry/sipxprovision/auto");
    }

    protected void tearDown() {

        System.getProperties().remove("conf.dir");
        assertNull(System.getProperty("conf.dir"));
    }

    /**
     * Checks that the 'loadProperties' method doesn't throw exceptions.
     *
     * @see Configuration#loadProperties(String)
     */
    public void testLoadPropertiesFailures() {

        assertNull(System.getProperty("conf.dir"));

        assertNull(Configuration.loadProperties("no such file"));

        System.setProperty("conf.dir", "/");

        assertNull(Configuration.loadProperties("no such file"));

        assertNull(Configuration.loadProperties("tmp"));
    }

    /**
     * Checks that the 'loadProperties' method works.
     *
     * @see Configuration#loadProperties(String)
     */
    public void testLoadPropertiesSuccess() {

        useDefaultConfigFileDirectory();

        Properties props = Configuration.loadProperties("/sipxprovision-config");

        assertNotNull(props);
        assertEquals(7, props.size());
        assertEquals("5440", props.getProperty("provision.servlet.port"));
    }

    /**
     * Checks that the 'dumpConfiguration' method hides passwords.
     *
     * @see Configuration#dumpConfiguration(PrintWriter)
     */
    public void testPasswordsHiddenInDump() {

        useDefaultConfigFileDirectory();

        Configuration apc = new Configuration();

        apc.dumpConfiguration(new PrintWriter(System.err, true));

        ByteArrayOutputStream stream = new ByteArrayOutputStream();
        apc.dumpConfiguration(new PrintWriter(stream));

        assertTrue(0 != stream.toString().length());
        assertEquals(-1, stream.toString().indexOf("HideMe"));
    }

    /**
     * Checks that Configuration produces the correct configuration values.
     *
     * @see Configuration
     */
    public void testConfiguration() {

        useDefaultConfigFileDirectory();

        Configuration apc = new Configuration();

        // domain-config (some include constants)
        assertEquals("fun.pm", apc.getSipDomainName());
        assertEquals(Configuration.SUPERADMIN_USERNAME + ":HideMe_during_dumpConfiguration",
                apc.getConfigurationRestCredentials());

        // sipxprovision-config (some include constants)
        assertEquals("DEBUG", apc.getLog4jLevel());
        assertEquals("/home/sipxchange/WORKING/INSTALL/var/log/sipxpbx/sipxprovision.log", apc.getLogfile());
        assertEquals(false, apc.isDebugOn());
        assertEquals(5440, apc.getServletPort());
        assertEquals("~~id~sipXprovision", apc.getProvisionSipUsername());
        assertEquals("HideMe_during_dumpConfiguration", apc.getProvisionSipPassword());
        assertEquals("https://dhcp-202.fun.pm:8443/sipxconfig", apc.getConfigurationUri());
    }
}
