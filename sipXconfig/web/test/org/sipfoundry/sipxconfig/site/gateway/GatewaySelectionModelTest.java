/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.gateway;

import java.util.ArrayList;
import java.util.List;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.gateway.Gateway;
import org.sipfoundry.sipxconfig.gateway.acme.AcmeGateway;

/**
 * GatewaySelectionModelTest
 */
public class GatewaySelectionModelTest extends TestCase {
    static private final String[] NAMES = {
        "a", "b", "c"
    };
    private GatewaySelectionModel m_model;

    protected void setUp() throws Exception {
        List gateways = new ArrayList();
        for (int i = 0; i < NAMES.length; i++) {
            String name = NAMES[i];
            Gateway gateway = new AcmeGateway();
            gateway.setUniqueId();
            gateway.setName(name);
            gateways.add(gateway);
        }
        m_model = new GatewaySelectionModel();
        m_model.setGateways(gateways);
    }

    public void testSetGateways() {
        try {
            m_model.setGateways(new ArrayList());
            fail("Should have thrown the exception");

        } catch (IllegalStateException e) {
            // this is expected
        }
    }

    public void testGetOptionCount() {
        assertEquals(NAMES.length, m_model.getOptionCount());
    }

    public void testGetOption() {
        Object option = m_model.getOption(2);
        assertTrue(option instanceof Gateway);
        Gateway g = (Gateway) option;
        assertEquals(NAMES[2], g.getName());
    }

    public void testGetLabel() {
        assertEquals(NAMES[1], m_model.getLabel(1));
    }

    public void testGetValue() {
        for (int i = 0; i < NAMES.length; i++) {
            String value = m_model.getValue(i);
            Object object = m_model.translateValue(value);
            assertTrue(object instanceof Gateway);
            Gateway g = (Gateway) object;
            assertEquals(NAMES[i], g.getName());
        }
    }
}
