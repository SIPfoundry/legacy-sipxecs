/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.restserver;

import static org.junit.Assert.assertEquals;

import java.io.StringWriter;

import org.apache.commons.io.IOUtils;
import org.junit.Test;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.domain.Domain;
import org.sipfoundry.sipxconfig.test.TestHelper;


public class RestServerConfigurationTest {
    
    @Test
    public void testConfig() throws Exception {
        RestConfiguration config = new RestConfiguration();
        config.setVelocityEngine(TestHelper.getVelocityEngine());
        RestServerSettings settings = new RestServerSettings();
        settings.setModelFilesContext(TestHelper.getModelFilesContext());
        Domain domain = new Domain("example.org");
        Location location = TestHelper.createDefaultLocation();
        StringWriter actual = new StringWriter();
        config.write(actual, settings, location, domain);
        String expected = IOUtils.toString(getClass().getResourceAsStream("expected-sipxrest-config"));
        assertEquals(expected, actual.toString());
    }
}
