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

import org.springframework.context.ApplicationEvent;

/**
 * Database task event that gets sent around once when business objects
 * need initialization in the database.  Initialization tasks are considered
 * completed on successfully sending the event, no need to mark the task complete.
 *
 * Example:
 * <code>
 * public class MySpringLoadedClass implements ApplicationListener {
 *    if (event instanceof InitializationTask) {
 *        InitializationTask task = (InitializationTask) event;
 *        if (task().equals("foo")) {
 *            doSomethingRelatedToFoo();
 *        }
 *    }
 * }
 * </code>
 */
public class InitializationTask extends ApplicationEvent {

    private static final Object SOURCE = new Object();

    private String m_task;

    public InitializationTask() {
        super(SOURCE);
    }

    public InitializationTask(String task) {
        super(SOURCE);
        setTask(task);
    }

    public String getTask() {
        return m_task;
    }

    public void setTask(String task) {
        m_task = task;
    }

    public int hashCode() {
        return m_task.hashCode();
    }

    public boolean equals(Object o) {
        if (o == null || !(o instanceof InitializationTask)) {
            return false;
        }
        boolean equal = m_task.equals(((InitializationTask) o).m_task);
        return equal;
    }
}
