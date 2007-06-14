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

import java.util.List;

import org.sipfoundry.sipxconfig.IntegrationTestCase;

public class SbcManagerImplTestIntegration extends IntegrationTestCase {
    private SbcManager m_sbcManager;

    public void testCreateDefaultSbc() throws Exception {
        assertEquals(0, countRowsInTable("sbc"));
        Sbc sbc = m_sbcManager.loadDefaultSbc();
        assertNotNull(sbc);
        assertFalse(sbc.isEnabled());
        assertNotNull(sbc.getRoutes());
        assertFalse(sbc.isNew());
        flush();
        assertEquals(1, countRowsInTable("sbc"));
    }

    public void testLoadDefaultSbc() throws Exception {
        loadDataSet("admin/dialplan/sbc/sbc.db.xml");
        assertEquals(3, countRowsInTable("sbc"));
        Sbc sbc = m_sbcManager.loadDefaultSbc();
        assertNotNull(sbc);
        assertEquals("10.1.2.3", sbc.getAddress());
        assertTrue(sbc.isEnabled());
        SbcRoutes routes = sbc.getRoutes();
        assertNotNull(routes);
        assertEquals("10.1.2.3/24", routes.getSubnets().get(0));
        assertEquals("*.example.org", routes.getDomains().get(0));
        assertEquals("*.example.net", routes.getDomains().get(1));
    }

    public void testLoadAuxSbc() throws Exception {
        loadDataSet("admin/dialplan/sbc/sbc.db.xml");
        assertEquals(3, countRowsInTable("sbc"));
        List<AuxSbc> sbcs = m_sbcManager.loadAuxSbcs();
        assertEquals(2, sbcs.size());
    }

    public void testSaveSbc() throws Exception {
        assertEquals(0, countRowsInTable("sbc"));
        Sbc sbc = m_sbcManager.loadDefaultSbc();

        sbc.setAddress("20.1.1.1");
        sbc.getRoutes().addDomain();
        sbc.getRoutes().getDomains().set(0, "*.example.us");

        m_sbcManager.saveSbc(sbc);

        flush();

        assertEquals("D", jdbcTemplate.queryForObject("select sbc_type from sbc", null,
                String.class));
        assertEquals("*.example.us", jdbcTemplate.queryForObject(
                "select domain from sbc_route_domain", null, String.class));
        assertEquals("20.1.1.1", jdbcTemplate.queryForObject("select address from sbc", null,
                String.class));
    }

    public void testSaveAuxSbc() throws Exception {
        assertEquals(0, countRowsInTable("sbc"));
        Sbc sbc = new AuxSbc();
        sbc.setRoutes(new SbcRoutes());

        sbc.setAddress("20.1.1.1");
        sbc.getRoutes().addDomain();
        sbc.getRoutes().getDomains().set(0, "*.example.us");

        m_sbcManager.saveSbc(sbc);

        flush();

        assertEquals(1, countRowsInTable("sbc"));
        assertEquals("A", jdbcTemplate.queryForObject("select sbc_type from sbc", null,
                String.class));
        assertEquals("*.example.us", jdbcTemplate.queryForObject(
                "select domain from sbc_route_domain", null, String.class));
        assertEquals("20.1.1.1", jdbcTemplate.queryForObject("select address from sbc", null,
                String.class));
    }

    public void setSbcManager(SbcManager sbcManager) {
        m_sbcManager = sbcManager;
    }
}
