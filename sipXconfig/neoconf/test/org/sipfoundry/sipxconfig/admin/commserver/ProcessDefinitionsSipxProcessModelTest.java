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

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessModel.ProcessName;
import org.sipfoundry.sipxconfig.test.TestUtil;

public class ProcessDefinitionsSipxProcessModelTest extends TestCase {
    private final static String CONFIG_DIR = "process.d";
    private final static String BOGUS_NAME = "SIPBogus";
    private String m_configDirectory;

    private final static ProcessName[] SERVICES = new ProcessName[] {
        ProcessName.REGISTRAR, ProcessName.MEDIA_SERVER, ProcessName.PRESENCE_SERVER,
        ProcessName.PROXY, ProcessName.ACD_SERVER
    };

    protected void setUp() throws Exception {
        m_configDirectory = TestUtil.getTestSourceDirectory(getClass()) + "/" + CONFIG_DIR;
    }

    public void testProcessDefinitionsModel() {

        ProcessDefinitionsSipxProcessModel model = null;
        try {
            model = new ProcessDefinitionsSipxProcessModel(m_configDirectory);
        } catch (Exception ex) {
            fail("Not supposed to throw exceptions");
        }

        assertNotNull(model.getAll());
        assertEquals(0 < model.getAll().size(), true);

        SimpleSipxProcessModel processModel = new SimpleSipxProcessModel();
        assertEquals(model.getAll().size() - processModel.getAll().size(), 0);
    }

    public void testGetProcess() {
        ProcessDefinitionsSipxProcessModel processModel = new ProcessDefinitionsSipxProcessModel(
                m_configDirectory);

        for (int x = 0; x < SERVICES.length; x++) {
            Process proc1 = processModel.getProcess(SERVICES[x]);
            Process proc2 = processModel.getProcess(SERVICES[x].getName());
            assertNotNull(proc1);
            assertNotNull(proc2);
            assertEquals(true, proc1.equals(proc2));
        }

        assertNotNull(processModel.getProcess(BOGUS_NAME));

        SimpleSipxProcessModel model = new SimpleSipxProcessModel();
        assertNull(model.getProcess(BOGUS_NAME));
    }

    public void testGetRestartable() {
        SimpleSipxProcessModel processModel = new SimpleSipxProcessModel();
        ProcessDefinitionsSipxProcessModel model = new ProcessDefinitionsSipxProcessModel(
                m_configDirectory);

        assertEquals(model.getRestartable().size() - processModel.getRestartable().size(), 0);
        assertEquals(model.getRestartable().size() > 1, true);

        for (Iterator it = model.getRestartable().iterator(); it.hasNext();) {
            Process proc = (Process) it.next();
            boolean equals = !BOGUS_NAME.equals(proc.getName());
            assertEquals(processModel.getRestartable().contains(proc), equals);
        }

        assertEquals(model.getRestartable().contains(new Process(BOGUS_NAME)), true);
        assertEquals(processModel.getRestartable().contains(new Process(BOGUS_NAME)), false);
        
        assertEquals(model.getRestartable().contains(new Process(ProcessName.CONFIG_SERVER)),
                false);
        assertEquals(model.getRestartable().contains(new Process(ProcessName.KEEP_ALIVE)), false);
    }
}
