/*
 * Copyright (C) 2012 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.snmp;

import static org.junit.Assert.assertEquals;

import java.io.IOException;
import java.io.StringWriter;
import java.util.Arrays;
import java.util.List;

import org.junit.Test;

public class SnmpConfigTest {
    
    @Test
    public void config() throws IOException {
        SnmpConfig config = new SnmpConfig();
        List<ProcessDefinition> defs = Arrays.asList(new ProcessDefinition("jay"), new ProcessDefinition("robin", ".*whatever.*"));
        StringWriter actual = new StringWriter();
        config.writeProcesses(actual, defs);
        assertEquals("regexp_proc jay\nregexp_proc robin 0 1 .*whatever.*\n", actual.toString());
    }
}
