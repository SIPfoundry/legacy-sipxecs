/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.mwi;

import static org.junit.Assert.assertEquals;

import java.io.StringWriter;
import java.util.Arrays;

import org.apache.commons.io.IOUtils;
import org.junit.Test;
import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.domain.Domain;
import org.sipfoundry.sipxconfig.ivr.Ivr;
import org.sipfoundry.sipxconfig.test.TestHelper;

public class MwiConfigTest {
    
    @Test
    public void testConfig() throws Exception {
        MwiConfig config = new MwiConfig();
        Location location = TestHelper.createDefaultLocation();
        Location additionalLocation = new Location();
        additionalLocation.setAddress("2.2.2.2");
        MwiSettings settings = new MwiSettings();
        settings.setModelFilesContext(TestHelper.getModelFilesContext());
        Domain domain = new Domain("example.org");
        domain.setSipRealm("grapefruit");
        StringWriter actual = new StringWriter();
        String expected = IOUtils.toString(getClass().getResourceAsStream("expected-status-config"));
        config.write(actual, settings, location, Arrays.asList(location, additionalLocation), domain);
        assertEquals(expected, actual.toString());
    }
    
    @Test    
    public void testPluginConfig() throws Exception {
        MwiConfig config = new MwiConfig();
        config.setVelocityEngine(TestHelper.getVelocityEngine());
        StringWriter actual = new StringWriter();
        String expected = IOUtils.toString(getClass().getResourceAsStream("expected-status-plugin-config"));        
        config.writePlugin(actual, new Address(Ivr.REST_API, "ivr.example.org", 100));
        assertEquals(expected, actual.toString());
    }
}
