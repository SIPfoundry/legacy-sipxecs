/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */

package org.sipfoundry.sipxconfig.cfgmgt;
import static org.junit.Assert.assertEquals;

import java.io.IOException;
import java.io.StringWriter;

import org.junit.Test;
import org.sipfoundry.sipxconfig.cfgmgt.KeyValueConfiguration;

public class KeyValueConfigurationTest {
    
    @Test
    public void config() throws IOException {
        StringWriter actual = new StringWriter();
        KeyValueConfiguration f = new KeyValueConfiguration(actual, " DELIM ", "PREFIX.");
        f.write("KEY", "VALUE");
        assertEquals("PREFIX.KEY DELIM VALUE\n", actual.toString());
    }
}
