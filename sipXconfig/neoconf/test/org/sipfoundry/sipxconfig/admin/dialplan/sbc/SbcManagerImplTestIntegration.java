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
        assertEquals(1, countRowsInTable("sbc"));
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

    public void testSaveSbc() throws Exception {
        assertEquals(0, countRowsInTable("sbc"));
        Sbc sbc = m_sbcManager.loadDefaultSbc();

        sbc.setAddress("20.1.1.1");
        sbc.getRoutes().addDomain();
        sbc.getRoutes().getDomains().set(0, "*.example.us");

        m_sbcManager.saveDefaultSbc(sbc);

        flush();

        assertEquals("*.example.us", jdbcTemplate.queryForObject(
                "select domain from sbc_route_domain", null, String.class));
        assertEquals("20.1.1.1", jdbcTemplate.queryForObject("select address from sbc", null,
                String.class));
    }

    public void setSbcManager(SbcManager sbcManager) {
        m_sbcManager = sbcManager;
    }
}
