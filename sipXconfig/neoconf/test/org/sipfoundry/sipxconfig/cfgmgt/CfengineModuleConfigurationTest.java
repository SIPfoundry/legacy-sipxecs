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
package org.sipfoundry.sipxconfig.cfgmgt;

import static org.junit.Assert.assertEquals;

import java.io.IOException;
import java.io.StringWriter;
import java.util.Arrays;

import org.apache.commons.io.IOUtils;
import org.junit.Test;


public class CfengineModuleConfigurationTest {
    
    @Test
    public void config() throws IOException {
        StringWriter actual = new StringWriter();
        CfengineModuleConfiguration config = new CfengineModuleConfiguration(actual);
        config.writeClass("goose", true);
        config.writeClass("gander", false);
        config.write("eggs", "white");
        config.writeList("transport", Arrays.asList("water", "land", "air"));
        String expected = IOUtils.toString(getClass().getResourceAsStream("expected.cfdat"));
        assertEquals(expected, actual.toString());
    }
}
