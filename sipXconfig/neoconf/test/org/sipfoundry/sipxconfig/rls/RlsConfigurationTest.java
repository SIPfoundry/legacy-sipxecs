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
package org.sipfoundry.sipxconfig.rls;

import static org.junit.Assert.assertEquals;

import java.io.StringWriter;

import org.apache.commons.io.IOUtils;
import org.junit.Test;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.domain.Domain;
import org.sipfoundry.sipxconfig.test.TestHelper;


public class RlsConfigurationTest {
    
    @Test
    public void testConfig() throws Exception {
        RlsConfig config = new RlsConfig();
        RlsSettings settings = new RlsSettings();
        settings.setModelFilesContext(TestHelper.getModelFilesContext());
        Domain domain = new Domain("example.org");
        domain.setSipRealm("realm.example.org");
        Location location = TestHelper.createDefaultLocation();
        StringWriter actual = new StringWriter();
        config.write(actual, settings, location, domain);
        String expected = IOUtils.toString(getClass().getResourceAsStream("expected-rls-config"));
        assertEquals(expected, actual.toString());
    }
    
    // resource lists xml is covered by ResourceListsTest
}
