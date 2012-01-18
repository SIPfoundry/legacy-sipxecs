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
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.domain.Domain;
import org.sipfoundry.sipxconfig.freeswitch.FreeswitchSettings;
import org.sipfoundry.sipxconfig.test.TestHelper;

public class SofiaConfigurationTest {
    private SofiaConfiguration m_configuration;
    
    @Before
    public void setUp() {
        m_configuration = new SofiaConfiguration();       
        m_configuration.setVelocityEngine(TestHelper.getVelocityEngine());
    }
    
    @Test
    public void config() throws IOException {
        StringWriter actual = new StringWriter();
        FreeswitchSettings settings = new FreeswitchSettings();
        settings.setModelFilesContext(TestHelper.getModelFilesContext());
        User userMedia = new User();
        userMedia.setUserName("~media-user");
        userMedia.setSipPassword("p4ssw0rd");
        Domain domain = new Domain("example.org");
        domain.setSipRealm("realm.example.com");
        m_configuration.write(actual, settings, domain, userMedia);
        String expected = IOUtils.toString(getClass().getResourceAsStream("sofia.conf.test.xml"));
        assertEquals(expected, actual.toString());        
    }
}
