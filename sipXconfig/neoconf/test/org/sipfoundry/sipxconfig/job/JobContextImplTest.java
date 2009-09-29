/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.job;

import java.io.Serializable;
import java.util.Date;
import java.util.List;

import junit.framework.TestCase;

public class JobContextImplTest extends TestCase {

    private JobContextImpl m_context;

    protected void setUp() throws Exception {
        m_context = new JobContextImpl();
        m_context.setMaxJobs(50);
        m_context.init();
    }

    public void testSchedule() {
        assertEquals(0, m_context.getJobs().size());

        Serializable jobId = m_context.schedule("abc");
        assertEquals(1, m_context.getJobs().size());

        Job job = m_context.getJobs().get(0);
        assertEquals("abc", job.getName());

        assertEquals(JobStatus.SCHEDULED, job.getStatus());

        m_context.start(jobId);
        job = m_context.getJobs().get(0);
        assertEquals(JobStatus.IN_PROGRESS, job.getStatus());

        Date during = new Date();

        m_context.success(jobId);
        job = m_context.getJobs().get(0);
        assertEquals(JobStatus.COMPLETED, job.getStatus());

        assertFalse("job start is after during", job.getStart().after(during));
        assertFalse("job stop is before during", job.getStop().before(during));

        assertEquals(0, job.getErrorMsg().length());
    }

    public void testRemoveCompleted() {
        Serializable[] jobIds = new Serializable[4];
        for (int i = 0; i < jobIds.length; i++) {
            jobIds[i] = m_context.schedule("test" + i);
            m_context.start(jobIds[i]);
        }

        m_context.success(jobIds[2]);
        m_context.failure(jobIds[3], null, null);

        int removed = m_context.removeCompleted();

        assertEquals(1, removed);

        List<Job> jobs = m_context.getJobs();
        assertEquals(3, jobs.size());

        for (Job job : jobs) {
            assertTrue(job.getName().startsWith("test"));
            assertFalse(job.getName().endsWith("2"));
        }
    }

    public void testClear() {
        Serializable[] jobIds = new Serializable[4];
        for (int i = 0; i < jobIds.length; i++) {
            jobIds[i] = m_context.schedule("test" + i);
            m_context.start(jobIds[i]);
        }

        m_context.success(jobIds[2]);
        assertFalse(m_context.isFailure());
        m_context.failure(jobIds[3], null, null);
        assertTrue(m_context.isFailure());

        List jobs = m_context.getJobs();
        m_context.clear();
        // clear does not affect retrieved jobs
        assertFalse(m_context.isFailure());

        assertEquals(4, jobs.size());
        jobs = m_context.getJobs();
        // but the list should be empty next time we retrieve it
        assertEquals(0, jobs.size());
    }

    public void testNullId() {
        // it should be OK to call most of context functions with null id
        m_context.start(null);
        m_context.success(null);
        m_context.failure(null, null, null);
        assertFalse(m_context.isFailure());
    }

    public void testFull() {
        final int max = 5;
        JobContextImpl context = new JobContextImpl();
        context.setMaxJobs(max);
        context.init();

        Serializable first = null;
        for (int i = 0; i < max; i++) {
            Serializable id = context.schedule("job" + i);
            assertNotNull(id);
            context.start(id);
            if (i == 0) {
                first = id;
            }
        }
        assertFalse(context.isFailure());

        final List<Job> before = context.getJobs();
        assertEquals(max, before.size());
        assertTrue(before.contains(first));

        // it should be OK now
        assertNotNull(context.schedule("extra job"));

        // but the queue do not grow
        final List<Job> after = context.getJobs();
        assertEquals(max, after.size());
        assertFalse(after.contains(first));

        // but it still should be OK to call success (or failure)
        context.failure(first, null, null);
        assertFalse(context.isFailure());
    }


    public void testAgeFailure() {
        final int max = 2;
        JobContextImpl context = new JobContextImpl();
        context.setMaxJobs(max);
        context.init();

        Serializable job0 = context.schedule("job1");
        context.start(job0);
        context.success(job0);
        assertFalse(context.isFailure());

        Serializable job1 = context.schedule("job1");
        context.start(job1);
        context.failure(job1, null, null);
        assertTrue(context.isFailure());

        Serializable job2 = context.schedule("job2");
        context.start(job2);
        context.success(job2);
        assertTrue(context.isFailure());

        context.schedule("job3");
        assertFalse(context.isFailure());
    }
}
