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
import org.sipfoundry.sipxconfig.test.TestHelper;


public class LocalStreamConfigurationTest {
    private LocalStreamConfiguration m_configuration;
    
    @Before
    public void setUp() {
        m_configuration = new LocalStreamConfiguration();
        m_configuration.setDocDir("/test/doc/dir");
        m_configuration.setVelocityEngine(TestHelper.getVelocityEngine());
    }
    
    @Test
    public void configCustomPrompts() throws IOException {
        StringWriter actual = new StringWriter();
        m_configuration.write(actual, "/test/moh/dir", true);
        String expected = IOUtils.toString(getClass().getResourceAsStream("local_stream.conf.test.xml"));
        assertEquals(expected, actual.toString());        
    }

    @Test
    public void configDefault() throws IOException {
        StringWriter actual = new StringWriter();
        String test = null;
        m_configuration.write(actual, test, false);
        String expected = IOUtils.toString(getClass().getResourceAsStream("local_stream_default.conf.test.xml"));
        assertEquals(expected, actual.toString());        
    }
}
