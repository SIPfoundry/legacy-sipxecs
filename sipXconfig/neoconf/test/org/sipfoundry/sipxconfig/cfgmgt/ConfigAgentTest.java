/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.cfgmgt;

import org.junit.Before;
import org.junit.Test;
import org.sipfoundry.sipxconfig.test.TestHelper;

public class ConfigAgentTest {
    private ConfigAgent m_agent;
    
    @Before
    public void setUp() {
        m_agent = new ConfigAgent();
    }
    
    @Test
    public void run() {
        m_agent.setLogDir(TestHelper.getTestOutputDirectory());
        m_agent.run("/bin/echo hello");
    }

    @Test(expected=ConfigException.class)
    public void timeout() {
        m_agent.setTimeout(250); // in millis
        m_agent.setLogDir(TestHelper.getTestOutputDirectory());
        m_agent.run("/bin/sleep 3");
    }
}
