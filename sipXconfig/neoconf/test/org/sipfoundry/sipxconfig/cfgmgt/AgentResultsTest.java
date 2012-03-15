/**
 *
 *
 * Copyright (c) 2010 / 2011 eZuce, Inc. All rights reserved.
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

import java.io.ByteArrayInputStream;
import java.io.InputStream;
import java.util.List;

import org.junit.Test;


public class AgentResultsTest {
    AgentResults m_results = new AgentResults();
    
    @Test
    public void parse() throws InterruptedException {
        InputStream dat = new ByteArrayInputStream("hello".getBytes()); 
        m_results.parse(dat);
        List<String> results = m_results.getResults(1000);
        assertEquals(1, results.size());
        assertEquals("hello", results.get(0));
    }
}
