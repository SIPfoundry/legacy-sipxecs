/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.cfgmgt;

import org.junit.Test;
import org.sipfoundry.sipxconfig.test.TestHelper;

public class ConfigAgentTest {
    
    @Test
    public void run() {
        ConfigAgent agent = new ConfigAgent();
        agent.setLogDir(TestHelper.getTestOutputDirectory());
        agent.run("sleep 1");
    }
}
