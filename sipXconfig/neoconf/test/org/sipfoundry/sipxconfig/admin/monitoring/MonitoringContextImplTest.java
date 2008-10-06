/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.monitoring;

import java.io.InputStream;
import java.util.List;

import junit.framework.TestCase;

import org.easymock.EasyMock;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.test.TestUtil;

public class MonitoringContextImplTest extends TestCase {
    private MonitoringContextImpl m_monitoringContextImpl;
    private LocationsManager m_locationsManager;
    private MRTGConfig m_mrtgConfig;
    private MRTGConfig m_mrtgTemplateConfig;

    @Override
    protected void setUp() {
        m_monitoringContextImpl = new MonitoringContextImpl();
        m_monitoringContextImpl.setEnabled(true);

        m_locationsManager = EasyMock.createNiceMock(LocationsManager.class);
        Location firstLocation = new Location();
        firstLocation.setFqdn("h1.sipfoundry.org");
        Location secondLocation = new Location();
        secondLocation.setFqdn("h2.sipfoundry.org");
        m_locationsManager.getLocations();
        EasyMock.expectLastCall().andReturn(new Location[] {firstLocation, secondLocation}).anyTimes();
        EasyMock.replay(m_locationsManager);

        m_monitoringContextImpl.setLocationsManager(m_locationsManager);

        m_mrtgConfig = new MRTGConfig(TestUtil.getTestSourceDirectory(getClass()) + "/" + "mrtg.cfg");
        m_mrtgTemplateConfig = new MRTGConfig(TestUtil.getTestSourceDirectory(getClass()) + "/" + "mrtg-t.cfg");
        m_monitoringContextImpl.setMrtgConfig(m_mrtgConfig);
        m_monitoringContextImpl.setMrtgTemplateConfig(m_mrtgTemplateConfig);
        try {
            m_monitoringContextImpl.afterPropertiesSet();
        } catch (Exception ex) {
            fail("could not initialize monitoring context");
        }
    }

    public void testGetAvailableHosts() {
        List<String> availableHosts = m_monitoringContextImpl.getAvailableHosts();
        assertEquals(2, availableHosts.size());
        assertEquals("h1.sipfoundry.org", availableHosts.get(0));
        assertEquals("h2.sipfoundry.org", availableHosts.get(1));
    }

    public void testGetMrtgWorkingDir() {
        assertEquals("/var/sipxdata/mrtg", m_monitoringContextImpl.getMrtgWorkingDir());
    }

    public void testGetTargetsFromTemplate() {
        assertEquals(6, m_monitoringContextImpl.getTargetsFromTemplate().size());

        List<MRTGTarget> templateTargets = m_monitoringContextImpl.getTargetsFromTemplate();
        assertEquals("cpuutil", templateTargets.get(0).getId());
        assertEquals("machine", templateTargets.get(1).getId());
        assertEquals("memgraph", templateTargets.get(2).getId());
        assertEquals("mempercent", templateTargets.get(3).getId());
        assertEquals("newconns", templateTargets.get(4).getId());
        assertEquals("estabcons", templateTargets.get(5).getId());
    }

    public void testGetHosts() {
        List<String> hosts = m_monitoringContextImpl.getHosts();
        assertEquals(2, hosts.size());
        assertEquals("localhost", hosts.get(0));
        assertEquals("192.168.0.27", hosts.get(1));
    }

    public void testGetTargetsForHost() {
        assertEquals(4, m_monitoringContextImpl.getTargetsForHost("localhost").size());
        assertEquals(1, m_monitoringContextImpl.getTargetsForHost("192.168.0.27").size());

        List<MRTGTarget> localhostTargets = m_monitoringContextImpl.getTargetsForHost("localhost");
        assertEquals("Server CPU Load", localhostTargets.get(0).getTitle());
        assertEquals("Free Memory", localhostTargets.get(1).getTitle());
        assertEquals("Currently Established TCP Connections", localhostTargets.get(2).getTitle());

        List<MRTGTarget> targets = m_monitoringContextImpl.getTargetsForHost("192.168.0.27");
        assertEquals("memgraph_192.168.0.27", targets.get(0).getId());
        assertEquals("memAvailReal.0&memTotalReal.0:sipxtest@192.168.0.27", targets.get(0).getExpression());
        assertEquals("Free Memory", targets.get(0).getTitle());
        assertEquals("<H1> Free Memory </H1>", targets.get(0).getPageTop());
        long expectedBytes = 10000000000L;
        assertEquals(expectedBytes, targets.get(0).getMaxBytes());
        assertEquals("B", targets.get(0).getShortLegend());
        assertEquals("Bytes", targets.get(0).getYLegend());
        assertEquals("Free", targets.get(0).getLegendI());
        assertEquals("Total", targets.get(0).getLegendO());
        assertEquals("Free memory (not including swap) in bytes", targets.get(0).getLegend1());
        assertEquals("Total memory", targets.get(0).getLegend2());
    }

    public void testGetReports() {
        assertEquals(4, m_monitoringContextImpl.getReports("localhost").size());
        assertEquals(1, m_monitoringContextImpl.getReports("192.168.0.27").size());

        List<String> reports = m_monitoringContextImpl.getReports("192.168.0.27");
        assertEquals("Free Memory", reports.get(0));

    }

    public void testGetMRTGTarget() {
        assertEquals("Server CPU Load", m_monitoringContextImpl.getMRTGTarget("Server CPU Load", "localhost")
                .getTitle());
        assertEquals("Currently Established TCP Connections", m_monitoringContextImpl.getMRTGTarget(
                "Currently Established TCP Connections", "localhost").getTitle());
        assertEquals("Free Memory", m_monitoringContextImpl.getMRTGTarget("Free Memory", "192.168.0.27").getTitle());
    }

    public void testIntializeConfigFiles() throws Exception {
        // This exposes XCF-2189
        m_mrtgConfig = new MRTGConfig(TestUtil.getTestSourceDirectory(getClass()) + "/" + "mrtg.cfg.test");
        m_monitoringContextImpl.setMrtgConfig(m_mrtgConfig);
        m_monitoringContextImpl.afterPropertiesSet();
    }

    public void testIntializeConfigFiles2() throws Exception {
        // If a target doesn't have underscore, it's not valid, so it should be eliminated
        InputStream mrtg_cfg = MonitoringContextImpl.class.getResourceAsStream("mrtg.cfg.test2");
        TestHelper.copyStreamToDirectory(mrtg_cfg, TestHelper.getTestDirectory(), "mrtg.cfg.test2");
        m_mrtgConfig = new MRTGConfig(TestHelper.getTestDirectory() + "/" + "mrtg.cfg.test2");
        m_monitoringContextImpl.setMrtgConfig(m_mrtgConfig);
        m_monitoringContextImpl.setMrtgStartupScript("echo");
        m_monitoringContextImpl.setMrtgNoOfTargetsFile(TestHelper.getTestDirectory() + "/" + "nr-targets");
        m_monitoringContextImpl.afterPropertiesSet();
        // should not have any targets => no hosts
        assertEquals(0, m_monitoringContextImpl.getHosts().size());
    }

    // TODO: a complex test will be to add the .rrd file and to check if the .pngs were created
    public void _testUpdateGraphs() {
        assertFalse(m_monitoringContextImpl.updateGraphs("localhost"));
    }
}
