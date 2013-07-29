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

import static org.sipfoundry.sipxconfig.components.LocalizationUtils.localizeException;
import static org.sipfoundry.sipxconfig.components.LocalizationUtils.localizeString;
import static org.sipfoundry.sipxconfig.components.TapestryUtils.createDateColumn;

import java.util.List;

import org.apache.commons.lang.StringUtils;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InitialValue;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.bean.EvenOdd;
import org.apache.tapestry.contrib.table.model.ITableColumn;
import org.apache.tapestry.services.ExpressionEvaluator;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.components.SipxBasePage;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.job.Job;
import org.sipfoundry.sipxconfig.job.JobContext;

/**
 * Displays current staus of background jobs
 */
public abstract class JobStatusPage extends SipxBasePage {
    public static final String PAGE = "admin/JobStatusPage";

    @Persist
    @InitialValue(value = "literal:failedJobs")
    public abstract String getTab();

    public abstract void setTab(String tab);

    @InjectObject(value = "spring:jobContext")
    public abstract JobContext getJobContext();

    @Bean
    public abstract EvenOdd getRowClass();

    @InjectObject(value = "service:tapestry.ognl.ExpressionEvaluator")
    public abstract ExpressionEvaluator getExpressionEvaluator();

    @Bean
    public abstract SipxValidationDelegate getValidator();

    public abstract List<Job> getNotFailedJobsProperty();

    public abstract void setNotFailedJobsProperty(List<Job> jobs);

    public abstract Job getNotFailedJob();

    public abstract List<Job> getFailedJobsProperty();

    public abstract void setFailedJobsProperty(List<Job> jobs);

    public abstract Job getFailedJob();

    public ITableColumn getStartColumn() {
        return createDateColumn("start", getMessages(), getExpressionEvaluator(), getPage().getLocale());
    }

    public ITableColumn getStopColumn() {
        return createDateColumn("stop", getMessages(), getExpressionEvaluator(), getPage().getLocale());
    }

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

    public String getLocationFqdn() {
        Job job = getFailedJob();
        if (job.getLocation() != null) {
            return job.getLocation().getFqdn();
        }
        return StringUtils.EMPTY;
    }

    public String getLocationFqdnNotFailed() {
        Job job = getNotFailedJob();
        if (job.getLocation() != null) {
            return job.getLocation().getFqdn();
        }
        return StringUtils.EMPTY;
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

    public void clear() {
        try {
            getJobContext().clear();
        } catch (UserException ex) {
            getValidator().record(ex, getMessages());
        }
        setNotFailedJobsProperty(null);
    }

    public void clearFailed() {
        try {
            getJobContext().clearFailed();
        } catch (UserException ex) {
            getValidator().record(ex, getMessages());
        }
        setFailedJobsProperty(null);
    }

    public void remove() {
        try {
            getJobContext().removeCompleted();
        } catch (UserException ex) {
            getValidator().record(ex, getMessages());
        }
        setNotFailedJobsProperty(null);
    }

}
