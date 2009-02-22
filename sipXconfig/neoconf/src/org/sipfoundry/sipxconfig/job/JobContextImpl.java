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
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import org.apache.commons.collections.buffer.CircularFifoBuffer;

public class JobContextImpl implements JobContext {
    private int m_maxJobs = 50;
    private CircularFifoBuffer m_jobs;
    private volatile boolean m_failure;

    public void init() {
        m_jobs = new CircularFifoBuffer(m_maxJobs);
    }

    public void setMaxJobs(int maxJobs) {
        m_maxJobs = maxJobs;
    }

    private synchronized Job getJob(Serializable id) {
        if (id instanceof Job && m_jobs.contains(id)) {
            return (Job) id;
        }
        return null;
    }

    private Serializable addNewJob(Job job) {
        job.setUniqueId();
        boolean recalculateFailure = m_failure && m_jobs.isFull();
        m_jobs.add(job);
        if (recalculateFailure) {
            m_failure = calculateFailure();
        }
        return job;
    }

    private boolean calculateFailure() {
        for (Iterator<Job> i = m_jobs.iterator(); i.hasNext();) {
            Job job = i.next();
            if (job.getStatus().equals(JobStatus.FAILED)) {
                return true;
            }
        }
        return false;
    }

    public synchronized Serializable schedule(String name) {
        Job job = new Job(name);
        return addNewJob(job);
    }

    public void start(Serializable jobId) {
        Job job = getJob(jobId);
        if (job != null) {
            job.start();
        }
    }

    public void success(Serializable jobId) {
        Job job = getJob(jobId);
        if (job != null) {
            job.success();
        }
    }

    public void failure(Serializable jobId, String errorMsg, Throwable exception) {
        Job job = getJob(jobId);
        if (job != null) {
            job.failure(errorMsg, exception);
            m_failure = true;
        }
    }

    public void warning(Serializable jobId, String warningMsg) {
        Job job = getJob(jobId);
        if (job != null) {
            job.warning(warningMsg);
        }
    }
    public synchronized int removeCompleted() {
        int counter = 0;
        for (Iterator<Job> i = m_jobs.iterator(); i.hasNext();) {
            Job job = i.next();
            if (job.getStatus().equals(JobStatus.COMPLETED)) {
                i.remove();
                counter++;
            }
        }
        return counter;
    }

    public synchronized void clear() {
        m_jobs.clear();
        m_failure = false;
    }

    public synchronized List<Job> getJobs() {
        return new ArrayList<Job>(m_jobs);
    }

    public boolean isFailure() {
        return m_failure;
    }
}
