/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.cfgmgt;

import static org.junit.Assert.assertEquals;

import java.io.ByteArrayInputStream;
import java.io.InputStream;
import java.util.List;

import org.junit.Test;


public class AgentResultsTest {
    AgentResults m_results = new AgentResults();
    
    @Test
    public void parse() {
        InputStream dat = new ByteArrayInputStream("hello".getBytes()); 
        m_results.parseInput(dat);
        List<String> results = m_results.getResults();
        assertEquals(1, results.size());
        assertEquals("hello", results.get(0));
    }
}
