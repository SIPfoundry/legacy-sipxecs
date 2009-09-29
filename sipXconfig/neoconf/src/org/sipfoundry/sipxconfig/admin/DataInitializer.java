/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.common.InitializationTask;
import org.sipfoundry.sipxconfig.common.SystemTaskEntryPoint;
import org.springframework.context.ApplicationContext;
import org.springframework.context.ApplicationContextAware;

/**
 * Iterates through all beans in Spring context and calls the beans that implement Patch interface
 * with opportunity to apply a db patch.
 */
public class DataInitializer implements SystemTaskEntryPoint, ApplicationContextAware {
    private static final Log LOG = LogFactory.getLog(DataInitializer.class);

    private AdminContext m_adminContext;
    private ApplicationContext m_app;

    public void runSystemTask(String[] args) {
        // set system property to prevent replication during initialization tasks
        System.setProperty("sipxconfig.initializationPhase", "true");

        String[] tasks = m_adminContext.getInitializationTasks();
        for (String task : tasks) {
            initializeData(task);
        }

        // unclear exactly why we'd need to ever call exit. If you find out why,
        // replace this comment w/reason
        if (args.length >= 2 && !"noexit".equals(args[1])) {
            System.exit(0);
        }
    }

    void initializeData(String task) {
        LOG.info("Creating task " + task);
        InitializationTask event = new InitializationTask(task);
        m_app.publishEvent(event);
        if (!task.equals("first-run")) {
            // HACK: do not delete first-run task
            m_adminContext.deleteInitializationTask(task);
        }
    }

    public void setApplicationContext(ApplicationContext applicationContext) {
        m_app = applicationContext;
    }

    public void setAdminContext(AdminContext adminContext) {
        m_adminContext = adminContext;
    }
}
