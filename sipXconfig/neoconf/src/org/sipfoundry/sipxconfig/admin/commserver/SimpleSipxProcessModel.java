/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.admin.commserver;

import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

public class SimpleSipxProcessModel implements SipxProcessModel {

    private static final Log LOG = LogFactory.getLog(SimpleSipxProcessModel.class);
    private HashMap<String, Process> m_map = new HashMap<String, Process>();
    private boolean m_isAdaptive; // false by default

    public SimpleSipxProcessModel() {
        initialize();
    }

    protected SimpleSipxProcessModel(boolean deferInit) {
        if (!deferInit) {
            initialize();
        }
    }

    protected void initialize() {
        Iterator it = ProcessName.getAll().iterator();
        for (; it.hasNext();) {
            ProcessName name = (ProcessName) it.next();
            m_map.put(name.getName(), new Process(name));
            LOG.debug("Initialize  process " + name.getName() + " added to the model");
        }
    }

    public void setIsAdaptive(boolean flag) {
        m_isAdaptive = flag;
    }

    public Process getProcess(String name) {
        Process p = m_map.get(name);
        if (p == null && m_isAdaptive) {
            p = new Process(name);
            m_map.put(name, p);
            LOG.debug("getProcess process " + p.getName() + " added to the model dynamically");
        }
        return p;
    }

    public Process getProcess(ProcessName name) {
        return getProcess(name.getName());
    }

    /**
     * Returns a collection of all Process objects. The collection is backed up by the internal
     * data structure, i.e. changes to the collection are reflected in the internal map and vise
     * versa
     * 
     * @return list of services in the model. The list is backed up by the internal map
     * 
     */
    protected Collection<Process> getAll() {
        return m_map.values();
    }

    /**
     * This should be used to get list of all the services except KEEP_ALIVE and CONFIG_SERVER
     * 
     * @return list of the services that you want usually restart
     * 
     */
    public List getRestartable() {
        Process[] noRestart = {
            new Process(ProcessName.KEEP_ALIVE), new Process(ProcessName.CONFIG_SERVER)
        };
        List<Process> processes = new LinkedList<Process>(getAll());
        processes.removeAll(Arrays.asList(noRestart));
        return Collections.unmodifiableList(processes);
    }

    protected void addProcess(String name) {
        LOG.debug("addProcess name=" + name);
        m_map.put(name, new Process(name));
    }

    protected void addProcess(Process proc) {
        LOG.debug("addProcess with name " + proc.getName());
        m_map.put(proc.getName(), proc);
    }
}
