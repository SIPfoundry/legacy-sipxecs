/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.freeswitch.config;

import static org.junit.Assert.assertEquals;

import java.io.IOException;
import java.io.StringWriter;

import org.apache.commons.io.IOUtils;
import org.junit.Before;
import org.junit.Test;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.freeswitch.FreeswitchSettings;
import org.sipfoundry.sipxconfig.test.TestHelper;



public class SwitchConfigurationTest {
    private SwitchConfiguration m_configuration;
    
    @Before
    public void setUp() {
        m_configuration = new SwitchConfiguration();       
        m_configuration.setVelocityEngine(TestHelper.getVelocityEngine());
    }
    
    @Test
    public void config() throws IOException {
        StringWriter actual = new StringWriter();
        FreeswitchSettings settings = new FreeswitchSettings();
        settings.setModelFilesContext(TestHelper.getModelFilesContext());
        settings.setSettingTypedValue("freeswitch-config/FREESWITCH_MAX_SESSIONS", 4000);
        settings.setSettingTypedValue("freeswitch-config/FREESWITCH_SESSION_PER_SECOND", 40);
        Location location = TestHelper.createDefaultLocation();
        m_configuration.write(actual, location, settings);
        String expected = IOUtils.toString(getClass().getResourceAsStream("switch.conf.test.xml"));
        assertEquals(expected, actual.toString());        
    }
}
