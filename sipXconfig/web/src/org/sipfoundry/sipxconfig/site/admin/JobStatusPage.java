/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.admin;

import java.util.List;

import org.apache.commons.lang.StringUtils;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.bean.EvenOdd;
import org.apache.tapestry.contrib.table.model.ITableColumn;
import org.apache.tapestry.services.ExpressionEvaluator;
import org.sipfoundry.sipxconfig.components.SipxBasePage;
import org.sipfoundry.sipxconfig.job.Job;
import org.sipfoundry.sipxconfig.job.JobContext;

import static org.sipfoundry.sipxconfig.components.LocalizationUtils.localizeException;
import static org.sipfoundry.sipxconfig.components.LocalizationUtils.localizeString;
import static org.sipfoundry.sipxconfig.components.TapestryUtils.createDateColumn;

/**
 * Displays current staus of background jobs
 */
public abstract class JobStatusPage extends SipxBasePage {
    public static final Object PAGE = "admin/JobStatusPage";

    @InjectObject(value = "spring:jobContext")
    public abstract JobContext getJobContext();

    @InjectObject(value = "service:tapestry.ognl.ExpressionEvaluator")
    public abstract ExpressionEvaluator getExpressionEvaluator();

    @Bean
    public abstract EvenOdd getRowClass();

    public abstract List<Job> getNotFailedJobsProperty();
    public abstract List<Job> getFailedJobsProperty();

    public abstract void setNotFailedJobsProperty(List<Job> jobs);
    public abstract void setFailedJobsProperty(List<Job> jobs);

    public abstract Job getNotFailedJob();
    public abstract Job getFailedJob();

    public List<Job> getNotFailedJobs() {
        List<Job> jobs = getNotFailedJobsProperty();
        if (jobs == null) {
            jobs = getJobContext().getNotFailedJobs();
            setNotFailedJobsProperty(jobs);
        }
        return jobs;
    }

    public List<Job> getFailedJobs() {
        List<Job> jobs = getFailedJobsProperty();
        if (jobs == null) {
            jobs = getJobContext().getFailedJobs();
            setFailedJobsProperty(jobs);
        }
        return jobs;
    }

    public void remove() {
        getJobContext().removeCompleted();
        setNotFailedJobsProperty(null);
    }

    public void clear() {
        getJobContext().clear();
        setNotFailedJobsProperty(null);
        setFailedJobsProperty(null);
    }

    public ITableColumn getStartColumn() {
        return createDateColumn("start", getMessages(), getExpressionEvaluator(), getPage().getLocale());
    }

    public ITableColumn getStopColumn() {
        return createDateColumn("stop", getMessages(), getExpressionEvaluator(), getPage().getLocale());
    }

    public String getFailedJobErrorMsg() {
        Job job = getFailedJob();
        if (!job.hasErrorMsg()) {
            return StringUtils.EMPTY;
        }
        StringBuilder error = new StringBuilder();

        String errorMsg = job.getRawErrorMsg();
        if (errorMsg != null) {
            error.append(localizeString(getMessages(), job.getRawErrorMsg()));
        }
        Throwable exception = job.getException();
        if (exception != null) {
            error.append('\n');
            error.append(localizeException(getMessages(), exception));
        }

        return error.toString();
    }
}
