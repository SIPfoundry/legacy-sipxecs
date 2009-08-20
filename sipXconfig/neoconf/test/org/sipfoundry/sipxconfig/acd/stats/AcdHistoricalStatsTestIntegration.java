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

import org.apache.commons.lang.time.DateUtils;
import org.postgresql.util.PGInterval;
import org.sipfoundry.sipxconfig.acd.AcdContext;
import org.sipfoundry.sipxconfig.acd.AcdServer;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.IntegrationTestCase;
import org.sipfoundry.sipxconfig.test.TestUtil;


public class AcdHistoricalStatsTestIntegration extends IntegrationTestCase {

    private AcdContext m_acdContext;
    private AcdHistoricalStats m_acdHistoricalStats;

    public void setAcdContext(AcdContext context) {
        m_acdContext = context;
    }

    public void testSignoutActivityReport() throws Exception {
        loadDataSetXml("admin/commserver/seedLocationsAndServices3.xml");
        List<AcdServer> acdServers = m_acdContext.getServers();
        assertEquals(1, acdServers.size());
        Location location = acdServers.get(0).getLocation();
        List<Map<String, Object>> stats = m_acdHistoricalStats.getReport("agentAvailablityReport", new Date(0), new Date(), location);
        assertEquals(4, stats.size());
        Map<String, Object> record;
        Iterator<Map<String, Object>> i = stats.iterator();
        record = i.next();
        assertEquals("sip:374@pingtel.com", record.get("agent_uri"));
    }

    public void testSignoutActivityReportSigninTime() throws Exception {
        loadDataSetXml("admin/commserver/seedLocationsAndServices3.xml");
        List<AcdServer> acdServers = m_acdContext.getServers();
        assertEquals(1, acdServers.size());
        Location location = acdServers.get(0).getLocation();
        List<Map<String, Object>> stats = m_acdHistoricalStats.getReport("agentAvailablityReport", new Date(0), new Date(), location);
        Map<String, Object> record;
        Iterator<Map<String, Object>> i = stats.iterator();
        record = i.next();
        // seed file is in UTC and so it should equal this date which is 5 hours from GMT
        Date expectedSigninTime = TestUtil.localizeDateTime("12/19/06 8:40:50 AM EST");
        Timestamp actualSigninTime = (Timestamp) record.get("sign_in_time");
        assertTrue(DateUtils.isSameInstant(expectedSigninTime, actualSigninTime));
    }

    public void testSignoutActivityReportColumns() throws Exception {
        loadDataSetXml("admin/commserver/seedLocationsAndServices3.xml");
        List<String> columns = m_acdHistoricalStats.getReportFields("agentAvailablityReport");
        assertEquals(3, columns.size());
        assertEquals("agent_uri", columns.get(0));
        assertEquals("sign_in_time", columns.get(1));
        assertEquals("sign_out_time", columns.get(2));
    }

    public void testForReportSQLErrors() throws Exception {
        loadDataSetXml("admin/commserver/seedLocationsAndServices3.xml");
        List<String> reports = m_acdHistoricalStats.getReports();
        for (String report : reports) {
            m_acdHistoricalStats.getReportFields(report);
            List<AcdServer> acdServers = m_acdContext.getServers();
            assertEquals(1, acdServers.size());
            Location location = acdServers.get(0).getLocation();
            m_acdHistoricalStats.getReport(report, new Date(0), new Date(), location);
        }
    }

    public void testAgentActivityReport() throws Exception {
        loadDataSetXml("admin/commserver/seedLocationsAndServices3.xml");
        List<AcdServer> acdServers = m_acdContext.getServers();
        assertEquals(1, acdServers.size());
        Location location = acdServers.get(0).getLocation();
        List<Map<String, Object>> stats = m_acdHistoricalStats.getReport("agentActivityReport", new Date(0), new Date(), location);
        Map<String, Object> record;
        Iterator<Map<String, Object>> i = stats.iterator();
        record = i.next();
        // Seed file has 2 complete calls for this uri, totaling 3:06
        assertEquals("sip:374@pingtel.com", record.get("agent_uri"));
        assertEquals(2L, record.get("num_calls"));
        PGInterval expectedDuration = new PGInterval(0,0,0,0,3,6);
        PGInterval actualDuration = (PGInterval)record.get("handle_duration");
        assertEquals(expectedDuration, actualDuration);
    }

    public void setAcdHistoricalStats(AcdHistoricalStats acdHistoricalStats) {
        m_acdHistoricalStats = acdHistoricalStats;
    }

}
