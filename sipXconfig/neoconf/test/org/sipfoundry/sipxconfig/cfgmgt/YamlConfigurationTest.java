/*
 * Copyright (C) 2012 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.cfgmgt;

import static org.junit.Assert.assertEquals;

import java.io.IOException;
import java.io.StringWriter;

import org.junit.Before;
import org.junit.Test;

public class YamlConfigurationTest {
    private StringWriter m_actual;
    private YamlConfiguration m_config;
    
    @Before
    public void setUp() {
        m_actual = new StringWriter();
        m_config = new YamlConfiguration(m_actual);        
    }
    
    @Test
    public void basic() throws IOException {
        m_config.write("bird", "robin");
        assertEquals("bird: robin\n", m_actual.toString());
    }

    @Test
    public void struct() throws IOException {
        m_config.startStruct("bird");
        m_config.write("name", "robin");
        m_config.write("species", "thrush");
        m_config.endStruct();
        assertEquals("bird:\n - name: robin\n   species: thrush\n", m_actual.toString());
    }

    @Test
    public void table() throws IOException {
        m_config.startStruct("birds");
        m_config.write("name", "robin");
        m_config.nextStruct();
        m_config.write("name", "bluejay");
        m_config.endStruct();
        String expected = "birds:\n - name: robin\n - name: bluejay\n";
        assertEquals(expected, m_actual.toString());
    }
}
