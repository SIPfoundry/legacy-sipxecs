/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.gateway;

import org.sipfoundry.sipxconfig.common.UserException;

import junit.framework.TestCase;

public class GatewayTest extends TestCase {

    public void testGetCallPattern() {
        Gateway gateway = new Gateway();
        assertEquals("123", gateway.getCallPattern("123"));
        gateway.setPrefix("99");
        assertEquals("99123", gateway.getCallPattern("123"));
    }

    public void testPorts() {
        GatewayModel model = new GatewayModel();
        model.setMaxPorts(2);
        Gateway gateway = new Gateway();
        gateway.setModel(model);

        FxoPort port1 = new FxoPort();
        FxoPort port2 = new FxoPort();
        FxoPort port3 = new FxoPort();

        assertEquals(0, gateway.getPorts().size());
        gateway.addPort(port1);
        assertEquals(1, gateway.getPorts().size());
        gateway.addPort(port2);
        assertEquals(2, gateway.getPorts().size());

        try {
            gateway.addPort(port3);
            fail("Should throw exception");
        } catch (UserException e) {
            assertEquals(2, gateway.getPorts().size());
        }

        gateway.removePort(port1);
        assertEquals(1, gateway.getPorts().size());

        assertSame(port2, gateway.getPorts().get(0));
    }

}
