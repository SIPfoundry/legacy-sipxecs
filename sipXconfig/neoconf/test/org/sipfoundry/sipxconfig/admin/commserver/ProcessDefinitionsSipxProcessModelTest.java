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

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessModel.ProcessName;
import org.sipfoundry.sipxconfig.test.TestUtil;

public class ProcessDefinitionsSipxProcessModelTest extends TestCase {
    private final static String BOGUS_NAME = "SIPBogus";
    private final static String m_configDirectory = TestUtil
            .getTestSourceDirectory(ProcessDefinitionsSipxProcessModelTest.class)
            + "/process.d";

    /** number of files in CONFIG_DIR */
    private final static int FILES_COUNT = 13;

    private final static ProcessName[] SERVICES = new ProcessName[] {
        ProcessName.REGISTRAR, ProcessName.MEDIA_SERVER, ProcessName.PRESENCE_SERVER, ProcessName.PROXY,
        ProcessName.ACD_SERVER
    };

    public void testProcessDefinitionsModel() {
        ProcessDefinitionsSipxProcessModel model = new ProcessDefinitionsSipxProcessModel(m_configDirectory);
        assertNotNull(model.getAll());
        assertEquals(FILES_COUNT, model.getAll().size());
    }

    public void testSimpleProcessModel() {
        SimpleSipxProcessModel processModel = new SimpleSipxProcessModel();
        assertEquals(ProcessName.getAll().size(), processModel.getAll().size());
    }

    public void testGetProcess() {
        ProcessDefinitionsSipxProcessModel processModel = new ProcessDefinitionsSipxProcessModel(m_configDirectory);

        for (int x = 0; x < SERVICES.length; x++) {
            Process proc1 = processModel.getProcess(SERVICES[x]);
            Process proc2 = processModel.getProcess(SERVICES[x].getName());
            assertNotNull(proc1);
            assertNotNull(proc2);
            assertEquals(proc1, proc2);
        }

        assertNotNull(processModel.getProcess(BOGUS_NAME));

        SimpleSipxProcessModel model = new SimpleSipxProcessModel();
        assertNull(model.getProcess(BOGUS_NAME));
    }

    public void testGetRestartable() {
        ProcessDefinitionsSipxProcessModel fileModel = new ProcessDefinitionsSipxProcessModel(m_configDirectory);

        // 2 processes CONFIG_SERVER and KEEP_ALIVE are not restartable
        assertEquals(FILES_COUNT - 2, fileModel.getRestartable().size());
        assertTrue(fileModel.getRestartable().size() > 1);

        // file model and simple model restartable are the same - with the exception of the bogus
        // process
        SimpleSipxProcessModel simpleModel = new SimpleSipxProcessModel();
        for (Process proc : fileModel.getRestartable()) {
            boolean isBogus = BOGUS_NAME.equals(proc.getName());
            assertEquals(simpleModel.getRestartable().contains(proc), !isBogus);
        }

        assertTrue(fileModel.getRestartable().contains(new Process(BOGUS_NAME)));
        assertFalse(simpleModel.getRestartable().contains(new Process(BOGUS_NAME)));

        assertFalse(fileModel.getRestartable().contains(new Process(ProcessName.CONFIG_SERVER)));
        assertFalse(fileModel.getRestartable().contains(new Process(ProcessName.KEEP_ALIVE)));
    }
}
