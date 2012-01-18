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


public class CfengineModuleConfigurationTest {
    
    @Test
    public void config() throws IOException {
        StringWriter actual = new StringWriter();
        CfengineModuleConfiguration config = new CfengineModuleConfiguration(actual);
        config.writeClass("goose", true);
        config.writeClass("gander", false);
        config.write("eggs", "white");
        assertEquals("+goose\n-gander\n=eggs=white\n", actual.toString());
    }
}
