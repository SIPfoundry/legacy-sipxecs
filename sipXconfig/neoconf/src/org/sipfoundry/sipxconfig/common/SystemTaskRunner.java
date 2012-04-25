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

import org.springframework.beans.factory.access.BeanFactoryLocator;
import org.springframework.beans.factory.access.BeanFactoryReference;
import org.springframework.context.ApplicationContext;
import org.springframework.context.access.ContextSingletonBeanFactoryLocator;

/**
 * Triggers system to start up, then find a requested bean, then runs it.
 */
public class SystemTaskRunner {

    public static void main(String[] args) {
        try {
            if (args == null || args.length == 0) {
                throw new IllegalArgumentException("bean to run is required as first argument");
            }
            new SystemTaskRunner().runMain(args);

        // need to exit, otherwise call from cfengine fails to return
        } catch (Exception e) {
            e.printStackTrace();
            System.exit(1);
        } finally {
            System.exit(0);
        }
    }

    void runMain(String[] args) {
        BeanFactoryLocator bfl = ContextSingletonBeanFactoryLocator.getInstance();
        BeanFactoryReference bfr = bfl.useBeanFactory("servicelayer-context");
        ApplicationContext app = (ApplicationContext) bfr.getFactory();
        SystemTaskEntryPoint task = (SystemTaskEntryPoint) app.getBean(args[0]);
        task.runSystemTask(args);
    }
}
