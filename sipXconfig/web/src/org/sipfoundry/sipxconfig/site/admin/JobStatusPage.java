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

import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.bean.EvenOdd;
import org.apache.tapestry.contrib.table.model.ITableColumn;
import org.apache.tapestry.html.BasePage;
import org.apache.tapestry.services.ExpressionEvaluator;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.job.Job;
import org.sipfoundry.sipxconfig.job.JobContext;

/**
 * Displays current staus of background jobs
 */
public abstract class JobStatusPage extends BasePage {
    public static final Object PAGE = "JobStatusPage";

    @InjectObject(value = "spring:jobContext")
    public abstract JobContext getJobContext();

    @InjectObject(value = "service:tapestry.ognl.ExpressionEvaluator")
    public abstract ExpressionEvaluator getExpressionEvaluator();

    @Bean
    public abstract EvenOdd getRowClass();

    public abstract List<Job> getJobsProperty();

    public abstract void setJobsProperty(List<Job> jobs);

    public abstract Job getJob();

    public List<Job> getJobs() {
        List<Job> jobs = getJobsProperty();
        if (jobs == null) {
            jobs = getJobContext().getJobs();
            setJobsProperty(jobs);
        }
        return jobs;
    }

    public void remove() {
        getJobContext().removeCompleted();
        setJobsProperty(null);
    }

    public void clear() {
        getJobContext().clear();
        setJobsProperty(null);
    }

    public ITableColumn getStartColumn() {
        return TapestryUtils.createDateColumn("start", getMessages(), getExpressionEvaluator(),
                getPage().getLocale());
    }

    public ITableColumn getStopColumn() {
        return TapestryUtils.createDateColumn("stop", getMessages(), getExpressionEvaluator(),
                getPage().getLocale());
    }
}
