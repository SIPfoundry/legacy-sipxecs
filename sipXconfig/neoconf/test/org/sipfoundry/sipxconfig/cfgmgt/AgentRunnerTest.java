/**
 *
 *
 * Copyright (c) 2010 / 2011 eZuce, Inc. All rights reserved.
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


import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.eq;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.isA;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

import java.io.ByteArrayOutputStream;
import java.io.Serializable;

import org.apache.commons.io.output.NullOutputStream;
import org.junit.Before;
import org.junit.Test;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.job.Job;
import org.sipfoundry.sipxconfig.job.JobContext;

public class AgentRunnerTest {
    private AgentRunner m_agent;
    
    @Before
    public void setUp() {
        m_agent = new AgentRunner();
    }
    
    @Test
    public void normal() {
        m_agent.runCommand("/bin/echo", new NullOutputStream());
    }

    @Test(expected=ConfigException.class)
    public void timeout() {
        m_agent.setTimeout(250); // in millis
        m_agent.runCommand("/bin/sleep 3", new NullOutputStream());
    }

    @Test
    public void err() {
        ByteArrayOutputStream actual = new ByteArrayOutputStream();
        int rc = m_agent.runCommand("/bin/ls /completely/bogus/dir", actual);
        assertEquals(2, rc);
        assertTrue("Got " + actual.toString(), actual.toString().contains("/completely/bogus/dir"));
    }
    
    @Test
    public void runNormalJob() {
        Location l = new Location("one", "1.1.1.1");
        JobContext jobc = createMock(JobContext.class);
        Serializable job = new Job("x");
        jobc.schedule("test", l);
        expectLastCall().andReturn(job).once();
        jobc.start(job);
        expectLastCall().once();
        jobc.success(job);
        expectLastCall().once();
        replay(jobc);
        m_agent.setJobContext(jobc);
        m_agent.runJob(l, "test", "/bin/echo");
        verify(jobc);        
    }   

    @Test
    public void runBadJob() {
        Location l = new Location("one", "1.1.1.1");
        JobContext jobc = createMock(JobContext.class);
        
        // expect 2 errors:  exit code and stderr results 
        Serializable job1 = new Job("1");        
        jobc.schedule("test", l);
        expectLastCall().andReturn(job1).times(2);
        jobc.start(job1);
        expectLastCall().times(2);
        jobc.failure(eq(job1), isA(String.class), isA(RuntimeException.class));
        expectLastCall().times(2);
        
        replay(jobc);
        
        m_agent.setJobContext(jobc);
        m_agent.runJob(l, "test", "/bin/ls /completely/bogus/dir");
        verify(jobc);        
    }   
}
