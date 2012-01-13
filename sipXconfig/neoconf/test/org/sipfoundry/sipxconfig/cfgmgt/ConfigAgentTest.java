/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.cfgmgt;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.createNiceMock;
import static org.easymock.EasyMock.eq;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.notNull;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;
import static org.junit.Assert.assertTrue;

import org.junit.Before;
import org.junit.Test;
import org.sipfoundry.sipxconfig.job.Job;
import org.sipfoundry.sipxconfig.job.JobContext;
import org.sipfoundry.sipxconfig.test.TestHelper;

public class ConfigAgentTest {
    private JobContext m_jobContext;
    private ConfigAgent m_agent;
    private Job m_job;
    
    @Before
    public void setUp() {
        m_jobContext = createNiceMock(JobContext.class);
        m_agent = new ConfigAgent();
        m_agent.setJobContext(m_jobContext);        
        m_job = new Job("test");
        m_job.start();
    }
    
    @Test
    public void run() {
        m_agent.setLogDir(TestHelper.getTestOutputDirectory());
        m_agent.run(m_job, "/bin/echo hello");
    }

    @Test
    public void timeout() {
        JobContext jobContext = createMock(JobContext.class);
        jobContext.start(eq(m_job));
        expectLastCall().once();
        jobContext.failure(eq(m_job), (String) notNull(), (Throwable) notNull());
        expectLastCall().once();
        replay(jobContext);        
        m_agent.setJobContext(jobContext);
        m_agent.setTimeout(250); // in millis
        m_agent.setLogDir(TestHelper.getTestOutputDirectory());
        m_agent.run(m_job, "/bin/sleep 3");
        verify(jobContext);
    }
}
