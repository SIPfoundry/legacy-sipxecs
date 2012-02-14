/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.cfgmgt;

import org.apache.commons.io.output.NullOutputStream;
import org.junit.Before;
import org.junit.Test;

public class ConfigAgentTest {
    private ConfigAgent m_agent;
    
    @Before
    public void setUp() {
        m_agent = new ConfigAgent();
    }
    
    @Test
    public void run() {
        m_agent.run("/bin/echo hello", new NullOutputStream());
    }

    @Test(expected=ConfigException.class)
    public void timeout() {
        m_agent.setTimeout(250); // in millis
        m_agent.run("/bin/sleep 3", new NullOutputStream());
    }
}
