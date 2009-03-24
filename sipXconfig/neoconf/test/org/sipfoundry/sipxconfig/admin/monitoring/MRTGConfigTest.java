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

import java.io.File;
import java.io.FileInputStream;
import java.util.ArrayList;
import java.util.List;

import junit.framework.TestCase;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.test.TestUtil;

public class MRTGConfigTest extends TestCase {
    private MRTGConfig m_mrtgConfig;
    private File m_mrtgConfigFile;

    protected void setUp() throws Exception {
        File mrtgTempDir = TestUtil.createTempDir("mrtg-temp");
        FileInputStream mrtgCfgStream = new FileInputStream(TestUtil.getTestSourceDirectory(getClass()) + "/" + "mrtg.cfg");
        TestHelper.copyStreamToDirectory(mrtgCfgStream, mrtgTempDir.getAbsolutePath(), "mrtg.cfg");
        m_mrtgConfigFile = new File(mrtgTempDir, "mrtg.cfg");
        m_mrtgConfig = new MRTGConfig(m_mrtgConfigFile.toString());

        try {
            m_mrtgConfig.parseConfig();
            m_mrtgConfig.setWorkingDir("/mrtg");
        } catch (Exception ex) {
            // could not initialize monitoring context, tests will fail
        }
    }

    public void testGetMRTGConfigTargets() {
        assertEquals(m_mrtgConfigFile.toString(), m_mrtgConfig
                .getFilename());
        assertEquals(4, m_mrtgConfig.getTargets().size());
        assertEquals("/mrtg", m_mrtgConfig.getWorkingDir());
        assertEquals("/mrtg/thresh", m_mrtgConfig.getThreshDir());
        assertEquals("rrdtool", m_mrtgConfig.getLogFormat());
        assertEquals("/usr/bin", m_mrtgConfig.getPathAdd());

        List<MRTGTarget> targets = m_mrtgConfig.getTargets();
        MRTGTarget cpuutil = targets.get(0);
        assertEquals("cpuutil_localhost", cpuutil.getId());
        assertEquals("ssCpuRawUser.0&ssCpuRawUser.0:sipxtest@localhost", cpuutil.getExpression());
        assertEquals("Server CPU Load", cpuutil.getTitle());
        assertEquals("<H1>Server CPU Load</H1>", cpuutil.getPageTop());
        assertEquals(100, cpuutil.getMaxBytes());
        assertEquals("%", cpuutil.getShortLegend());
        assertEquals("CPU Utilization", cpuutil.getYLegend());
        assertEquals("Used", cpuutil.getLegendI());
        assertEquals("Current CPU percentage load", cpuutil.getLegend1());
        assertEquals("ymwd", cpuutil.getUnscaled());
        assertTrue(cpuutil.growRight());
        assertTrue(cpuutil.noPercent());
        assertFalse(cpuutil.gauge());
        assertFalse(cpuutil.bits());
        assertFalse(cpuutil.transparent());
        assertFalse(cpuutil.perMinute());

        MRTGTarget swap = targets.get(1);
        assertEquals("swap_localhost", swap.getId());
        assertEquals(".1.3.6.1.4.1.2021.4.3.0&.1.3.6.1.4.1.2021.4.4.0:sipxtest@localhost", swap.getExpression());
        assertEquals("Swap Usage", swap.getTitle());
        assertEquals(10000000000L, swap.getMaxBytes());
        assertEquals("B", swap.getShortLegend());
        assertEquals("Available Swap", swap.getYLegend());
        assertEquals("Available Swap", swap.getLegendI());
        assertEquals("Used Swap", swap.getLegendO());
        assertEquals("Used Swap", swap.getLegend1());
        assertTrue(swap.growRight());
        assertTrue(swap.noPercent());
        assertTrue(swap.gauge());

        MRTGTarget tcpConnections = targets.get(3);
        assertEquals("estabcons_localhost", tcpConnections.getId());
        assertEquals("tcpCurrEstab.0&tcpCurrEstab.0:sipxtest@", tcpConnections.getExpression());
        assertEquals("Currently Established TCP Connections", tcpConnections.getTitle());
        assertEquals("<H1> Established TCP Connections </H1>", tcpConnections.getPageTop());
        assertEquals(10000000000L, tcpConnections.getMaxBytes());
        assertEquals("Connections", tcpConnections.getYLegend());
        assertEquals("In", tcpConnections.getLegendI());
        assertEquals("Established connections", tcpConnections.getLegend1());
        assertTrue(tcpConnections.growRight());
        assertTrue(tcpConnections.noPercent());
        assertTrue(tcpConnections.gauge());
        assertFalse(tcpConnections.bits());
        assertFalse(tcpConnections.transparent());
        assertFalse(tcpConnections.perMinute());
    }

    public void testGetMRTGConfigTargets2() {
        m_mrtgConfig = new MRTGConfig(TestUtil.getTestSourceDirectory(getClass()) + "/"
                + "mrtg.cfg.test");

        try {
            m_mrtgConfig.parseConfig();
            m_mrtgConfig.setWorkingDir("/mrtg");
        } catch (Exception ex) {
            // could not initialize monitoring context, tests will fail
        }
        assertEquals(TestUtil.getTestSourceDirectory(getClass()) + "/" + "mrtg.cfg.test", m_mrtgConfig
                .getFilename());
        assertEquals(0, m_mrtgConfig.getTargets().size());
        //MRTGTarget theTarget = m_mrtgConfig.getTargets().get(0);
        //assertEquals("", theTarget.getId().substring(0, theTarget.getId().indexOf("_")));
    }

    public void testGetMRTGConfigHosts() {
        assertEquals(2, m_mrtgConfig.getHosts().size());
        List<String> hosts = m_mrtgConfig.getHosts();
        assertEquals("localhost", hosts.get(0));
        assertEquals("192.168.0.27", hosts.get(1));
    }

    public void testMRTGConfigToString() {
        String eol = System.getProperty("line.separator");
        List<MRTGTarget> targets = new ArrayList<MRTGTarget>();
        targets.add(m_mrtgConfig.getTargets().get(0));
        m_mrtgConfig.setTargets(targets);
        String m_mrtgConfigString = StringUtils.remove(m_mrtgConfig.toString(), eol);
        String outputToCompare = "RunAsDaemon: Yes"
                + "NoDetach: Yes"
                + "Interval: 5"
                + "workdir: /mrtg"
                + "threshdir: /mrtg/thresh"
                + "LoadMibs: /usr/share/snmp/mibs/UCD-SNMP-MIB.txt, /usr/share/snmp/mibs/TCP-MIB.txt"
                + "EnableIPv6: no"
                + "LogFormat: rrdtool"
                + "PathAdd: /usr/bin"
                + "## Server CPU Load#target[cpuutil_localhost]: ssCpuRawUser.0&ssCpuRawUser.0:sipxtest@localhosttitle[cpuutil_localhost]: Server CPU Loadpagetop[cpuutil_localhost]: <H1>Server CPU Load</H1>maxbytes[cpuutil_localhost]: 100shortlegend[cpuutil_localhost]: %ylegend[cpuutil_localhost]: CPU Utilizationlegendi[cpuutil_localhost]: Usedlegend1[cpuutil_localhost]: Current CPU percentage loadunscaled[cpuutil_localhost]: ymwdoptions[cpuutil_localhost]: growright,nopercent";
        assertEquals(m_mrtgConfigString, outputToCompare);
    }

    public void testMRTGConfigToStringHeader() {
        List<MRTGTarget> targets = new ArrayList<MRTGTarget>();
        targets.add(new MRTGTarget());
        m_mrtgConfig.setTargets(targets);
        String m_mrtgConfigString = StringUtils.remove(m_mrtgConfig.toString(), System
                .getProperty("line.separator"));
        String outputToCompare =  "RunAsDaemon: Yes"
                + "NoDetach: Yes"
                + "Interval: 5"
                + "workdir: /mrtg"
                + "threshdir: /mrtg/thresh"
                + "LoadMibs: /usr/share/snmp/mibs/UCD-SNMP-MIB.txt, /usr/share/snmp/mibs/TCP-MIB.txt"
                + "EnableIPv6: no"
                + "LogFormat: rrdtool"
                + "PathAdd: /usr/bin"
                + "## #maxbytes[]: 10000000000";
        assertEquals(m_mrtgConfigString, outputToCompare);
    }

    public void testAddCustomTarget() {
        MRTGTarget customTarget = new MRTGTarget();
        customTarget.setId("custom");
        customTarget.setExpression("customExpresion");
        customTarget.setGroup("customGroup");
        customTarget.setTitle("customTitle");
        customTarget.setPageTop("customPageTop");
        customTarget.setMaxBytes(10);
        customTarget.setShortLegend("shortLegend");
        customTarget.setYLegend("YLegend");
        customTarget.setLegendI("ILegend");
        customTarget.setLegendO("OLegend");
        customTarget.setLegend1("Legend1");
        customTarget.setLegend2("Legend2");
        customTarget.setOptions("gauge,growright,nopercent");
        customTarget.setUnscaled("unscaled");
        customTarget.setkMG("kMG");
        customTarget.setFactor(4);
        customTarget.setKilo();

        String eol = System.getProperty("line.separator");
        List<MRTGTarget> targets = new ArrayList<MRTGTarget>();
        targets.add(customTarget);
        m_mrtgConfig.setTargets(targets);
        String m_mrtgConfigString = StringUtils.remove(m_mrtgConfig.toString(), eol);
        String outputToCompare =  "RunAsDaemon: Yes"
                + "NoDetach: Yes"
                + "Interval: 5"
                + "workdir: /mrtg"
                + "threshdir: /mrtg/thresh"
                + "LoadMibs: /usr/share/snmp/mibs/UCD-SNMP-MIB.txt, /usr/share/snmp/mibs/TCP-MIB.txt"
                + "EnableIPv6: no"
                + "LogFormat: rrdtool"
                + "PathAdd: /usr/bin"
                + "#target-group=customGroup## customTitle#target[custom]: customExpresiontitle[custom]: customTitlepagetop[custom]: customPageTopmaxbytes[custom]: 10shortlegend[custom]: shortLegendylegend[custom]: YLegendlegendi[custom]: ILegendlegendo[custom]: OLegendlegend1[custom]: Legend1legend2[custom]: Legend2unscaled[custom]: unscaledoptions[custom]: gauge,growright,nopercent";
        assertEquals(m_mrtgConfigString, outputToCompare);
    }

    public void testMRTGTargetOptions() {
        MRTGTarget customTarget = new MRTGTarget();
        customTarget.setOptions("gauge,growright,nopercent,bits,transparent,perminute");
        assertTrue(customTarget.gauge());
        assertTrue(customTarget.growRight());
        assertTrue(customTarget.noPercent());
        assertTrue(customTarget.gauge());
        assertTrue(customTarget.bits());
        assertTrue(customTarget.transparent());
        assertTrue(customTarget.perMinute());

        customTarget = new MRTGTarget();
        customTarget.setOptions("growright,nopercent,bits,perminute");
        assertFalse(customTarget.gauge());
        assertTrue(customTarget.growRight());
        assertTrue(customTarget.noPercent());
        assertFalse(customTarget.gauge());
        assertTrue(customTarget.bits());
        assertFalse(customTarget.transparent());
        assertTrue(customTarget.perMinute());
    }
}
