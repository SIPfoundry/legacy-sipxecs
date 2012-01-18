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
import org.sipfoundry.sipxconfig.domain.Domain;
import org.sipfoundry.sipxconfig.freeswitch.FreeswitchSettings;
import org.sipfoundry.sipxconfig.test.TestHelper;

public class FreeswitchConfigurationTest {
    FreeswitchConfiguration m_configuration;

    @Before
    public void setUp() {
        m_configuration = new FreeswitchConfiguration();  
        m_configuration.setVelocityEngine(TestHelper.getVelocityEngine());
    }

    @Test
    public void config() throws Exception {
        StringWriter actual = new StringWriter();
        Domain domain = new Domain("example.org");
        FreeswitchSettings settings = new FreeswitchSettings();
        settings.setModelFilesContext(TestHelper.getModelFilesContext());
        m_configuration.write(actual, domain, settings);
        String expected = IOUtils.toString(getClass().getResourceAsStream("freeswitch.test.xml"));
        assertEquals(expected, actual.toString());        
    }
}
