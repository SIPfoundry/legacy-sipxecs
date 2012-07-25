/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.cdr;

import static org.junit.Assert.assertEquals;

import java.io.StringWriter;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;

import org.apache.commons.io.IOUtils;
import org.junit.Before;
import org.junit.Test;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.test.TestHelper;

public class CdrConfigurationTest {
    private Location m_one;
    private Location m_two;
    private CdrSettings m_settings;
    private CdrConfiguration m_config;

    @Before
    public void setUp() {
        m_one = new Location();
        m_one.setFqdn("one.example.org");
        m_two = new Location();
        m_two.setFqdn("two.example.org");
        m_settings = new CdrSettings();
        m_settings.setModelFilesContext(TestHelper.getModelFilesContext());
        m_config = new CdrConfiguration();
    }

    @Test
    public void cseHosts() {
        String actual = CdrConfiguration.cseHosts(Arrays.asList(m_one, m_two), 100);
        assertEquals("one.example.org:100, two.example.org:100", actual);
    }

    @Test
    public void testConfig() throws Exception {
        StringWriter actual = new StringWriter();
        m_config.write(actual, Collections.singletonList(m_one), m_settings);
        String expected = IOUtils.toString(getClass().getResourceAsStream("expected-callresolver-config-no-ha"));
        assertEquals(expected, actual.toString());
    }

    @Test
    public void testHAConfig() throws Exception {
        List<Location> locations = Arrays.asList(m_one, m_two);
        StringWriter actual = new StringWriter();
        m_config.write(actual, locations, m_settings);
        String expected = IOUtils.toString(getClass().getResourceAsStream("expected-callresolver-config"));
        assertEquals(expected, actual.toString());
    }
}
