/**
 *
 *
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.cfgmgt;

import static org.junit.Assert.assertEquals;

import java.io.IOException;
import java.io.StringWriter;
import java.util.Arrays;
import java.util.List;

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
        m_config.endArray();
        assertEquals("bird:\n   name: robin\n   species: thrush\n", m_actual.toString());
    }

    @Test
    public void arrayOfStructs() throws IOException {
        m_config.startArray("bird");
        m_config.write("name", "robin");
        m_config.write("species", "thrush");
        m_config.endArray();
        assertEquals("bird:\n - name: robin\n   species: thrush\n", m_actual.toString());
    }
    
    @Test
    public void arrayOfElements() throws IOException {
        List<String> values = Arrays.asList("hearing gull", "northern cardinal", "scrub jay");
        m_config.writeArray("birds", values);
        String expected = "birds:\n - hearing gull\n - northern cardinal\n - scrub jay\n";
        assertEquals(expected, m_actual.toString());
    }    

    @Test
    public void table() throws IOException {
        m_config.startArray("birds");
        m_config.write("name", "robin");
        m_config.nextElement();
        m_config.write("name", "bluejay");
        m_config.endArray();
        String expected = "birds:\n - name: robin\n - name: bluejay\n";
        assertEquals(expected, m_actual.toString());
    }
}
