/**
 *
 *
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.registrar;

import static org.junit.Assert.assertEquals;

import java.io.StringWriter;

import org.apache.commons.io.IOUtils;
import org.junit.Test;
import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.domain.Domain;
import org.sipfoundry.sipxconfig.im.ImManager;
import org.sipfoundry.sipxconfig.parkorbit.ParkOrbitContext;
import org.sipfoundry.sipxconfig.presence.PresenceServer;
import org.sipfoundry.sipxconfig.proxy.ProxyManager;
import org.sipfoundry.sipxconfig.test.TestHelper;


public class RegistrarConfigurationTest {
    
    @Test
    public void testConfig() throws Exception {
        RegistrarConfiguration config = new RegistrarConfiguration();
        StringWriter actual = new StringWriter();
        RegistrarSettings settings = new RegistrarSettings();
        settings.setModelFilesContext(TestHelper.getModelFilesContext());
        Domain domain = new Domain("example.org");
        domain.setSipRealm("grapefruit");        
        Address imApi = new Address(ImManager.XMLRPC_ADDRESS, "im.example.org", 100);
        Address presenceApi = new Address(PresenceServer.HTTP_ADDRESS, "presence.example.org", 101);
        Address park = new Address(ParkOrbitContext.SIP_TCP_PORT, "park.example.org", 102);
        Address proxy = new Address(ProxyManager.TCP_ADDRESS, "proxy.example.org", 103);
        Location location = TestHelper.createDefaultLocation();
        config.write(actual, settings, domain, location, proxy, imApi, presenceApi, park);
        String expected = IOUtils.toString(getClass().getResourceAsStream("expected-registrar-config"));
        assertEquals(expected, actual.toString());
    }
}
