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

import java.sql.Timestamp;
import java.util.Date;
import java.util.Iterator;
import java.util.List;
import java.util.Map;

import junit.framework.TestCase;

import org.apache.commons.lang.time.DateUtils;
import org.postgresql.util.PGInterval;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.test.TestUtil;
import org.springframework.context.ApplicationContext;

public class AcdHistoricalStatsTestDb extends TestCase {
    
    private AcdHistoricalStats m_history;

    protected void setUp() throws Exception {
        ApplicationContext app = TestHelper.getApplicationContext();
        m_history = (AcdHistoricalStats) app.getBean(AcdHistoricalStats.BEAN_NAME);
    }
    
    public void testSignoutActivityReport() {
        List<Map<String, Object>> stats = m_history.getReport("agentAvailablityReport", new Date(0), new Date());        
        assertEquals(10, stats.size());
        Map<String, Object> record;
        Iterator<Map<String, Object>> i = stats.iterator();
        record = i.next();
        assertEquals("sip:374@pingtel.com", record.get("agent_uri"));
    }

    public void testSignoutActivityReportSigninTime() {
        List<Map<String, Object>> stats = m_history.getReport("agentAvailablityReport", new Date(0), new Date());        
        Map<String, Object> record;
        Iterator<Map<String, Object>> i = stats.iterator();
        record = i.next();
        // seed file is in UTC and so it should equal this date which is 5 hours from GMT
        Date expectedSigninTime = TestUtil.localizeDateTime("12/19/06 8:40:50 AM EST");
        Timestamp actualSigninTime = (Timestamp) record.get("sign_in_time");
        assertTrue(DateUtils.isSameInstant(expectedSigninTime, actualSigninTime));
    }

    public void testSignoutActivityReportColumns() {
        List<String> columns = m_history.getReportFields("agentAvailablityReport");        
        assertEquals(3, columns.size());
        assertEquals("agent_uri", columns.get(0));
        assertEquals("sign_in_time", columns.get(1));
        assertEquals("sign_out_time", columns.get(2));
    }
    
    public void testForReportSQLErrors() {
        List<String> reports = m_history.getReports();
        for (String report : reports) {
            m_history.getReportFields(report);
            m_history.getReport(report, new Date(0), new Date());
        }
    }
    
    public void testAgentActivityReport() {
        List<Map<String, Object>> stats = m_history.getReport("agentActivityReport", new Date(0), new Date());        
        Map<String, Object> record;
        Iterator<Map<String, Object>> i = stats.iterator();
        record = i.next();
        // Seed file has 3 complete calls for this uri, totaling 4:36
        assertEquals("sip:374@pingtel.com", record.get("agent_uri"));
        assertEquals(3L, record.get("num_calls"));
        PGInterval expectedDuration = new PGInterval(0,0,0,0,4,36);
        PGInterval actualDuration = (PGInterval)record.get("handle_duration");
        assertEquals(expectedDuration, actualDuration);
    }
}
