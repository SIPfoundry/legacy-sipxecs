/**
 *
 *
 * Copyright (c) 2010 / 2011 eZuce, Inc. All rights reserved.
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
