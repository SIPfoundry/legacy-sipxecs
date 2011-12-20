/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.presence;

import static org.junit.Assert.assertEquals;

import java.io.StringWriter;

import org.apache.commons.io.IOUtils;
import org.junit.Test;
import org.sipfoundry.sipxconfig.domain.Domain;
import org.sipfoundry.sipxconfig.test.TestHelper;

public class PresenceConfigurationTest {

    @Test
    public void testWrite() throws Exception {
        PresenceConfig config = new PresenceConfig();
        Domain domain = new Domain("example.org");
        domain.setSipRealm("grapefruit");        
        PresenceSettings settings = new PresenceSettings();        
        settings.setModelFilesContext(TestHelper.getModelFilesContext());
        StringWriter actual = new StringWriter();
        config.write(actual, settings, "10.1.1.1", domain);
        String expected = IOUtils.toString(getClass().getResourceAsStream("expected-presence-config"));
        assertEquals(expected, actual.toString());
    }
}
