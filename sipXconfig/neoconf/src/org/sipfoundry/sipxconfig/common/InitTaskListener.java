/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.common;

import java.util.List;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.springframework.context.ApplicationEvent;
import org.springframework.context.ApplicationListener;

/**
 * Special version of application listener triggered only by the task with a specified name.
 * InitTaskListener
 */
public abstract class InitTaskListener implements ApplicationListener {
    private static final Log LOG = LogFactory.getLog(InitTaskListener.class);

    private List<String> m_taskNames;

    public void setTaskNames(List<String> taskNames) {
        m_taskNames = taskNames;
    }

    public void onApplicationEvent(ApplicationEvent event) {
        if (event instanceof InitializationTask) {
            InitializationTask task = (InitializationTask) event;
            String taskName = task.getTask();
            if (m_taskNames == null) {
                throw new IllegalStateException("Task list should not be null.");
            }
            if (m_taskNames.contains(taskName)) {
                LOG.info("Initialization task " + taskName + " being handled by " + getClass().getName());
                onInitTask(taskName);
            }
        }
    }

    public abstract void onInitTask(String task);
}
