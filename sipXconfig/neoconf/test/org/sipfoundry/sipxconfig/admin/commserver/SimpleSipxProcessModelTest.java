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

import java.util.Iterator;
import java.util.List;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessModel.ProcessName;
import org.sipfoundry.sipxconfig.admin.commserver.Process;

public class SimpleSipxProcessModelTest extends TestCase {
    private SimpleSipxProcessModel m_processModel = new SimpleSipxProcessModel();
    private List m_processes;

    private final static ProcessName[] SERVICES = new ProcessName[] {
        ProcessName.REGISTRAR, ProcessName.MEDIA_SERVER, ProcessName.PRESENCE_SERVER,
        ProcessName.AUTH_PROXY, ProcessName.ACD_SERVER
    };

    protected void setUp() {
        m_processes = m_processModel.getRestartable();
    }
    
    public void testSimpleModel() {
        assertNotNull(m_processModel.getAll());
        assertEquals(0 < m_processModel.getAll().size(), true);
        assertEquals(m_processModel.getAll().size(), ProcessName.getAll().size());

        List<ProcessName> l = ProcessName.getAll();
        assertNotNull(l);
        assertEquals(l.size(), m_processModel.getAll().size());
        for (ProcessName name : l) {
            Process proc = m_processModel.getProcess(name.getName());
            assertNotNull(proc);
            assertEquals(proc.equals(new Process(name)), true);
        }
    }

    public void testGetProcess() {
        SimpleSipxProcessModel processModel = new  SimpleSipxProcessModel();
        
        for (int x = 0; x < SERVICES.length; x++) {
            Process proc1 = processModel.getProcess(SERVICES[x]);
            Process proc2 = processModel.getProcess(SERVICES[x].getName());
            assertNotNull(proc1);
            assertNotNull(proc2);
            assertEquals(true, proc1.equals(proc2));
        }
    }

    public void testGetProcessAdaptive() {
        SimpleSipxProcessModel processModel = new SimpleSipxProcessModel();
            
        String dummyName = new String("dummyname");
        assertNull(processModel.getProcess(dummyName));

        processModel.setIsAdaptive(true);
        
        // Add process dynamically 
        assertNotNull(processModel.getProcess(dummyName));
        Process dummy = processModel.getProcess(dummyName);
        assertNotNull(dummy);
        assertEquals(dummy.getName().equals(dummyName), true);
        
        assertNull(m_processModel.getProcess(dummyName));
    }
    
    public void testGetRestartable() {
        SimpleSipxProcessModel processModel = new SimpleSipxProcessModel();
        
        assertEquals(m_processes.size(), processModel.getRestartable().size());
        assertEquals(m_processes.size()>0, true);

        for (Iterator it = m_processes.iterator(); it.hasNext();) {
            Process proc = (Process)it.next();
            assertEquals(processModel.getRestartable().contains(proc), true);
        }

        assertEquals(processModel.getRestartable().contains(new Process(ProcessName.CONFIG_SERVER)), false);
        assertEquals(processModel.getRestartable().contains(new Process(ProcessName.KEEP_ALIVE)), false);
    }
    
    public void testGetRestartableAdaptive() {
        SimpleSipxProcessModel processModel = new SimpleSipxProcessModel();

        String dummyName = new String("dummyname");
        assertNull(processModel.getProcess(dummyName));

        processModel.setIsAdaptive(true);
        
        Process alsoDummy = new Process("dummyInMyTummy");

        // Check that changes in list returned by getRestartable()
        // do not impact the model
        List processes = processModel.getRestartable();
        try {
            processes.add(alsoDummy);
            fail("Unmodifiable list expected");
        } catch (UnsupportedOperationException exc) {
        }
        
        assertEquals(m_processes.size(), processModel.getRestartable().size());
        assertEquals(processes.contains(alsoDummy), false);
        assertEquals(processModel.getRestartable().contains(alsoDummy), false);
        
        // Add process dynamically 
        assertNotNull(processModel.getProcess(dummyName));
        
        assertEquals(m_processes.size()+1, processModel.getRestartable().size());
        for (Iterator it = m_processes.iterator(); it.hasNext();) {
            Process proc = (Process)it.next();
            assertEquals(processModel.getRestartable().contains(proc), true);
        }
        Process dummy = new Process(dummyName);
        assertEquals(processModel.getRestartable().contains(dummy), true);
        assertEquals(processModel.getRestartable().contains(alsoDummy), false);
        assertEquals(processes.contains(dummy), false);
    }
}
