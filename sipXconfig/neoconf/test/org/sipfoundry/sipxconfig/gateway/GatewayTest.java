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

    public void testClone() {
        Gateway gateway = new Gateway();
        gateway.setPrefix("99");
        gateway.setAddress("example.com");
        gateway.setAddressPort(5070);

        try {
            Gateway gw = (Gateway) gateway.clone();
            assertEquals("99123", gw.getCallPattern("123"));
            assertEquals("example.com", gw.getAddress());
            assertEquals(5070, gw.getAddressPort());
            assertEquals("example.com:5070", gw.getGatewayAddress());
            assertTrue(gw.equals(gateway));
        } catch (CloneNotSupportedException exc) {
            fail("Should throw exception");
        }
    }

    public void testGetGatewayAddress() {
        Gateway gateway = new Gateway();
        gateway.setPrefix("99");
        gateway.setAddress("example.com");
        assertEquals("example.com", gateway.getGatewayAddress());

        gateway.setAddressPort(5070);
        assertEquals("example.com:5070", gateway.getGatewayAddress());
    }

    public void testGetGatewayTransportUrlParams() {
        Gateway gateway = new Gateway();
        gateway.setPrefix("99");
        gateway.setAddress("example.com");
        assertNull(gateway.getGatewayTransportUrlParam());

        gateway.setAddressTransport(Gateway.AddressTransport.TCP);
        assertEquals("transport=tcp", gateway.getGatewayTransportUrlParam());

        gateway.setAddressTransport(Gateway.AddressTransport.UDP);
        assertEquals("transport=udp", gateway.getGatewayTransportUrlParam());

        gateway.setAddressTransport(Gateway.AddressTransport.NONE);
        assertNull(gateway.getGatewayTransportUrlParam());
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
