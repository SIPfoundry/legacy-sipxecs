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

import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.common.BeanWithId;

/**
 * Information about the job
 */
public class Job extends BeanWithId implements Serializable {
    private static final Log LOG = LogFactory.getLog(Job.class);

    private Date m_start;

    private Date m_stop;

    private JobStatus m_status = JobStatus.SCHEDULED;

    private final String m_name;

    private String m_errorMsg = StringUtils.EMPTY;

    private Throwable m_exception;

    public Job(String name) {
        m_name = name;
    }

    Job() {
        this(StringUtils.EMPTY);
    }

    public void start() {
        if (!m_status.equals(JobStatus.SCHEDULED)) {
            throw new IllegalStateException();
        }
        m_start = new Date();
        m_status = JobStatus.IN_PROGRESS;
    }

    public void success() {
        stop(JobStatus.COMPLETED);
    }

    public void warning(String warningMsg) {
        stop(JobStatus.WARNING);
        m_errorMsg = warningMsg;
        m_exception = null;
    }

    public void failure(String errorMsg, Throwable e) {
        stop(JobStatus.FAILED);
        m_errorMsg = errorMsg;
        m_exception = e;
    }

    private void stop(JobStatus status) {
        if (!m_status.equals(JobStatus.IN_PROGRESS)) {
            throw new IllegalStateException();
        }
        m_stop = new Date();
        m_status = status;
        if (LOG.isDebugEnabled()) {
            long interval = m_stop.getTime() - m_start.getTime();
            LOG.debug("Job: " + m_name + " finished " + m_status + " time: " + interval);
        }
    }

    public String getName() {
        return m_name;
    }

    public Date getStart() {
        return m_start;
    }

    public Date getStop() {
        return m_stop;
    }

    public JobStatus getStatus() {
        return m_status;
    }

    public String getErrorMsg() {

        if (!m_status.equals(JobStatus.FAILED) && !m_status.equals(JobStatus.WARNING)) {
            return org.apache.commons.lang.StringUtils.EMPTY;
        }
        StringBuffer error = new StringBuffer();
        if (m_errorMsg != null) {
            error.append(m_errorMsg);
        }
        if (m_exception != null) {
            error.append('\n');
            error.append(m_exception.getLocalizedMessage());
        }
        return error.toString();
    }

    public boolean hasErrorMsg() {
        return m_status == JobStatus.FAILED || m_status == JobStatus.WARNING;
    }

    public String getRawErrorMsg() {
        return m_errorMsg;
    }

    public Throwable getException() {
        return m_exception;
    }
}
