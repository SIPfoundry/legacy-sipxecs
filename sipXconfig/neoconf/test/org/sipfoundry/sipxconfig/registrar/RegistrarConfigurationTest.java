/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.registrar;

import static org.junit.Assert.assertEquals;

import java.io.StringWriter;

import org.apache.commons.io.IOUtils;
import org.junit.Test;
import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.domain.Domain;
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
        Address imApi = new Address("im.example.org", 100);
        Address presenceApi = new Address("presence.example.org", 101);
        Address park = new Address("park.example.org", 102);
        Address proxy = new Address("proxy.example.org", 103);
        Location location = TestHelper.createDefaultLocation();
        config.write(actual, settings, domain, location, proxy, imApi, presenceApi, park);
        String expected = IOUtils.toString(getClass().getResourceAsStream("expected-registrar-config"));
        assertEquals(expected, actual.toString());
    }
}
