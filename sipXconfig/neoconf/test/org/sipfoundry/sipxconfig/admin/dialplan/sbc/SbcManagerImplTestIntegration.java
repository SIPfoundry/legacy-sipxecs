/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.admin.dialplan.sbc;

import java.util.Arrays;
import java.util.List;

import org.sipfoundry.sipxconfig.IntegrationTestCase;

public class SbcManagerImplTestIntegration extends IntegrationTestCase {
    private static final String QUERY_ADDRESS = "select device.address from sbc_device device, sbc sbc "
            + " where sbc.sbc_device_id = device.sbc_device_id";

    private SbcManager m_sbcManager;
    private SbcDeviceManager m_sbcDeviceManager;

    public void testCreateDefaultSbc() throws Exception {
        loadDataSetXml("admin/dialplan/sbc/domain.xml");
        assertEquals(0, countRowsInTable("sbc"));
        Sbc sbc = m_sbcManager.loadDefaultSbc();
        assertNotNull(sbc);
        assertFalse(sbc.isEnabled());
        assertNotNull(sbc.getRoutes());
        assertEquals(3, sbc.getRoutes().getSubnets().size());
        assertEquals("*.example.org", sbc.getRoutes().getDomains().get(0));
        assertFalse(sbc.isNew());
        flush();
        assertEquals(1, countRowsInTable("sbc"));
    }

    public void testLoadDefaultSbc() throws Exception {
        loadDataSet("admin/dialplan/sbc/sbc-device.db.xml");
        loadDataSet("admin/dialplan/sbc/sbc.db.xml");
        assertEquals(3, countRowsInTable("sbc"));
        Sbc sbc = m_sbcManager.loadDefaultSbc();
        assertNotNull(sbc);
        assertNotNull(sbc.getSbcDevice());
        assertEquals("10.1.2.3", sbc.getSbcDevice().getAddress());
        assertTrue(sbc.isEnabled());
        SbcRoutes routes = sbc.getRoutes();
        assertNotNull(routes);
        assertEquals("10.1.2.3/24", routes.getSubnets().get(0));
        assertEquals("*.example.org", routes.getDomains().get(0));
        assertEquals("*.example.net", routes.getDomains().get(1));
    }

    public void testLoadAuxSbcs() throws Exception {
        loadDataSet("admin/dialplan/sbc/sbc-device.db.xml");
        loadDataSet("admin/dialplan/sbc/sbc.db.xml");
        assertEquals(3, countRowsInTable("sbc"));
        List<AuxSbc> sbcs = m_sbcManager.loadAuxSbcs();
        assertEquals(2, sbcs.size());
    }

    public void testRemoveAuxSbcs() throws Exception {
        loadDataSet("admin/dialplan/sbc/sbc-device.db.xml");
        loadDataSet("admin/dialplan/sbc/sbc.db.xml");
        assertEquals(3, countRowsInTable("sbc"));
        assertEquals(2, countRowsInTable("sbc_route_subnet"));
        m_sbcManager.removeSbcs(Arrays.asList(1001));
        flush();

        assertEquals(2, countRowsInTable("sbc"));
        assertEquals(1, countRowsInTable("sbc_route_subnet"));
    }

    public void testClear() throws Exception {
        loadDataSet("admin/dialplan/sbc/sbc-device.db.xml");
        loadDataSet("admin/dialplan/sbc/sbc.db.xml");
        assertEquals(3, countRowsInTable("sbc"));
        assertEquals(2, countRowsInTable("sbc_route_subnet"));
        m_sbcManager.clear();
        flush();

        assertEquals(0, countRowsInTable("sbc"));
        assertEquals(0, countRowsInTable("sbc_route_subnet"));
    }

    public void testLoadAuxSbc() throws Exception {
        loadDataSet("admin/dialplan/sbc/sbc-device.db.xml");
        loadDataSet("admin/dialplan/sbc/sbc.db.xml");
        assertEquals(3, countRowsInTable("sbc"));
        AuxSbc sbc = m_sbcManager.loadSbc(1001);

        assertNotNull(sbc);
        assertNotNull(sbc.getSbcDevice());
        assertEquals("10.1.2.4", sbc.getSbcDevice().getAddress());
        assertEquals("10.1.2.5/24", sbc.getRoutes().getSubnets().get(0));
        assertEquals(0, sbc.getRoutes().getDomains().size());
    }

    public void testSaveSbc() throws Exception {
        loadDataSetXml("admin/dialplan/sbc/domain.xml");
        loadDataSet("admin/dialplan/sbc/sbc-device.db.xml");
        assertEquals(0, countRowsInTable("sbc"));
        Sbc sbc = m_sbcManager.loadDefaultSbc();

        SbcDevice sbcDevice = m_sbcDeviceManager.getSbcDevice(1002);
        sbc.setSbcDevice(sbcDevice);
        sbc.getRoutes().getDomains().set(0, "*.example.us");

        m_sbcManager.saveSbc(sbc);

        flush();

        assertEquals("D", jdbcTemplate.queryForObject("select sbc_type from sbc", null,
                String.class));
        assertEquals("*.example.us", jdbcTemplate.queryForObject(
                "select domain from sbc_route_domain", null, String.class));
        assertEquals("10.1.2.5", jdbcTemplate.queryForObject(QUERY_ADDRESS, null, String.class));
    }

    public void testSaveAuxSbc() throws Exception {
        loadDataSet("admin/dialplan/sbc/sbc-device.db.xml");
        assertEquals(0, countRowsInTable("sbc"));
        Sbc sbc = new AuxSbc();
        sbc.setRoutes(new SbcRoutes());

        SbcDevice sbcDevice = m_sbcDeviceManager.getSbcDevice(1002);
        sbc.setSbcDevice(sbcDevice);
        sbc.getRoutes().addDomain();
        sbc.getRoutes().getDomains().set(0, "*.example.us");

        m_sbcManager.saveSbc(sbc);

        flush();

        assertEquals(1, countRowsInTable("sbc"));
        assertEquals("A", jdbcTemplate.queryForObject("select sbc_type from sbc", null,
                String.class));
        assertEquals("*.example.us", jdbcTemplate.queryForObject(
                "select domain from sbc_route_domain", null, String.class));
        assertEquals("10.1.2.5", jdbcTemplate.queryForObject(QUERY_ADDRESS, null, String.class));
    }

    public void testDeleteAssociateSbcDevice() throws Exception {
        loadDataSet("admin/dialplan/sbc/sbc-device.db.xml");
        loadDataSet("admin/dialplan/sbc/sbc.db.xml");
        assertEquals(3, countRowsInTable("sbc"));

        Sbc sbc = m_sbcManager.loadDefaultSbc();
        assertNotNull(sbc);
        assertNotNull(sbc.getSbcDevice());
        assertEquals("10.1.2.3", sbc.getSbcDevice().getAddress());
        assertTrue(sbc.isEnabled());
        m_sbcDeviceManager.deleteSbcDevice(sbc.getSbcDevice().getId());
        flush();
        assertNotNull(sbc);
        assertNull(sbc.getSbcDevice());
        assertFalse(sbc.isEnabled());
        assertEquals(3, countRowsInTable("sbc"));

        AuxSbc auxSbc = m_sbcManager.loadSbc(1001);
        assertNotNull(auxSbc);
        assertNotNull(auxSbc.getSbcDevice());
        assertEquals("10.1.2.4", auxSbc.getSbcDevice().getAddress());
        assertTrue(auxSbc.isEnabled());
        m_sbcDeviceManager.deleteSbcDevice(auxSbc.getSbcDevice().getId());
        flush();
        try {
            auxSbc = m_sbcManager.loadSbc(1001);
        } catch (Exception ex) {
            assertTrue(true);
        }
        assertEquals(2, countRowsInTable("sbc"));
    }

    public void testGetRoutes() throws Exception {
        loadDataSet("admin/dialplan/sbc/sbc-device.db.xml");
        loadDataSet("admin/dialplan/sbc/sbc.db.xml");

        SbcRoutes routes = m_sbcManager.getRoutes();
        assertNotNull(routes);
        assertTrue(routes.getDomains().contains("*.example.org"));
        assertTrue(routes.getDomains().contains("*.example.net"));
        assertTrue(routes.getSubnets().contains("10.1.2.3/24"));
        assertTrue(routes.getSubnets().contains("10.1.2.5/24"));
    }

    public void setSbcManager(SbcManager sbcManager) {
        m_sbcManager = sbcManager;
    }

    public void setSbcDeviceManager(SbcDeviceManager sbcDeviceManager) {
        m_sbcDeviceManager = sbcDeviceManager;
    }
}
