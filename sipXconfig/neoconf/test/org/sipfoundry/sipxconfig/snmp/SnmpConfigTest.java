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
package org.sipfoundry.sipxconfig.snmp;

import static org.junit.Assert.assertEquals;

import java.io.IOException;
import java.io.StringWriter;
import java.util.Arrays;
import java.util.List;

import org.apache.commons.io.FileUtils;
import org.apache.commons.io.IOUtils;
import org.junit.Test;

public class SnmpConfigTest {
    
    @Test
    public void config() throws IOException {
        SnmpConfig config = new SnmpConfig();
        List<ProcessDefinition> defs = Arrays.asList(ProcessDefinition.sipx("jay"), ProcessDefinition.sysvByRegex("robin", ".*whatever.*"));
        StringWriter actual = new StringWriter();
        config.writeProcesses(actual, defs);
        String expected = IOUtils.toString(getClass().getResourceAsStream("expected-config"));
        assertEquals(expected, actual.toString());
    }

    @Test
    public void configWithRestart() throws IOException {
        SnmpConfig config = new SnmpConfig();
        List<ProcessDefinition> defs = Arrays.asList(ProcessDefinition.sipxByRegex("robin", ".*whatever.*", "restart"));
        StringWriter actual = new StringWriter();
        config.writeProcesses(actual, defs);
        String expected = IOUtils.toString(getClass().getResourceAsStream("expected-config-restart"));
        assertEquals(expected, actual.toString());
    }
}
