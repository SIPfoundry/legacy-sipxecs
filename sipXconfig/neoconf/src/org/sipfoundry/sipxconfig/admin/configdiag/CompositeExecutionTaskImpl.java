/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.admin.configdiag;

import java.util.ArrayList;
import java.util.List;

public class CompositeExecutionTaskImpl implements CompositeExecutionTask {

    private List<ExecutionTask> m_tasks;

    public CompositeExecutionTaskImpl() {
        m_tasks = new ArrayList<ExecutionTask>();
    }

    public void execute() {
        for (ExecutionTask task : m_tasks) {
            task.execute();
        }
    }

    public void addExecutionTask(ExecutionTask task) {
        m_tasks.add(task);
    }

    public void removeAll() {
        m_tasks = new ArrayList<ExecutionTask>();
    }
}
