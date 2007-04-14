/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.acd.stats;

import java.io.IOException;
import java.io.StringWriter;
import java.util.Collections;
import java.util.Date;
import java.util.HashMap;
import java.util.List;
import java.util.Locale;
import java.util.Map;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.test.TestUtil;

public class AcdHistoricalStatsTest extends TestCase {
    
    public void testEnabled() {
        AcdHistoricalStatsImpl history = new AcdHistoricalStatsImpl();
        assertFalse(history.isEnabled());
        
        history.setReportScript(TestUtil.getTestSourceDirectory(getClass()) + "/Bogus");        
        assertFalse(history.isEnabled());
        
        history.setReportScript(TestUtil.getTestSourceDirectory(getClass()) + "/AcdHistoricalStatsTest.java");
        assertTrue(history.isEnabled());
        
        history.setEnabled(true);
        assertTrue(history.isEnabled());

        history.setEnabled(false);
        assertFalse(history.isEnabled());        
    }
    
    public void testDumpReport() throws IOException {
        AcdHistoricalStatsImpl history = new AcdHistoricalStatsImpl();
        List<Map<String, Object>> reportData = Collections.emptyList();
        history.dumpReport(null, reportData, null);
    }
    
    public void testDumpReportData() throws IOException {
        AcdHistoricalStatsImpl history = new AcdHistoricalStatsImpl();
        Map<String, Object> row0 = new HashMap<String, Object>();
        row0.put("dateSeen", new Date(0));
        row0.put("bird", "bluejay");
        List<Map<String, Object>> report = Collections.singletonList(row0);
        StringWriter writer = new StringWriter();
        history.dumpReport(writer, report, Locale.ENGLISH);
        assertEquals("bird,dateSeen\nbluejay,Wed, 31 Dec 1969 19:00:00 -0500\n", writer.toString());
    }
}
