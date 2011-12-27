/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.freeswitch.config;

import static org.junit.Assert.assertEquals;

import java.io.StringWriter;

import org.apache.commons.io.IOUtils;
import org.junit.Before;
import org.junit.Test;
import org.sipfoundry.sipxconfig.test.TestHelper;

public class ModulesConfigurationTest {
    
    private ModulesConfiguration m_configuration;
    
    @Before
    public void setUp() {
        m_configuration = new ModulesConfiguration();
        m_configuration.setVelocityEngine(TestHelper.getVelocityEngine());
    }

    @Test
    public void withG729() throws Exception {
        StringWriter actual = new StringWriter();
        m_configuration.write(actual, true);
        String expected = IOUtils.toString(getClass().getResourceAsStream("modules_g729.conf.test.xml"));
        assertEquals(expected, actual.toString());      
    }

    @Test
    public void withoutG729() throws Exception {
        StringWriter actual = new StringWriter();
        m_configuration.write(actual, false);
        String expected = IOUtils.toString(getClass().getResourceAsStream("modules.conf.test.xml"));
        assertEquals(expected, actual.toString());      
    }
}
