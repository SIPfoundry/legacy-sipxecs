/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.setting;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.InputStream;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.TestHelper;

public class ConfigFileStorageTest extends TestCase {
    static String CONFIG_FILE = "test-config.in";

    private static final String DEFAULT_VALUE = "83.51";

    private String m_configDirectory;

    private ConfigFileStorage m_storage;

    private SettingImpl m_andorra;

    private SettingImpl m_space;

    private SettingImpl m_bongo;

    protected void setUp() throws Exception {
        InputStream configStream = ConfigFileStorageTest.class.getResourceAsStream(CONFIG_FILE);
        m_configDirectory = TestHelper.getTestDirectory();
        TestHelper.copyStreamToDirectory(configStream, m_configDirectory, CONFIG_FILE);

        m_storage = new ConfigFileStorage(m_configDirectory);

        m_andorra = new SettingImpl();
        m_andorra.setProfileName(CONFIG_FILE);
        m_andorra.setName("Andorra");
        m_andorra.setValue(DEFAULT_VALUE);

        m_space = new SettingImpl();
        m_space.setProfileName(CONFIG_FILE);
        m_space.setName("extra_space");

        m_bongo = new SettingImpl();
        m_bongo.setProfileName(CONFIG_FILE);
        m_bongo.setName("bongo");
    }

    public void testGet() {

        assertEquals(DEFAULT_VALUE, m_storage.getValue(m_andorra));

        assertEquals(4, Integer.valueOf(m_storage.getValue(m_space)).intValue());

        assertNull(m_storage.getValue(m_bongo));
    }

    public void testPut() throws Exception {
        String newValue = "99.999";
        m_storage.setValue(m_andorra, newValue);
        assertEquals(newValue, m_storage.getValue(m_andorra));

        BufferedReader reader = new BufferedReader(new FileReader(new File(m_configDirectory,
                CONFIG_FILE)));

        m_storage.flush();
        boolean found = false;
        for (String line = reader.readLine(); line != null; line = reader.readLine()) {
            if (line.startsWith(m_andorra.getName())) {
                assertTrue(line.endsWith(newValue));
                found = true;
            }
        }
        assertTrue("Setting not found", found);
    }
    
    public void testReset() throws Exception {
        String newValue = "99.999";        
        m_storage.setValue(m_andorra, newValue);
        assertEquals(newValue, m_storage.getValue(m_andorra));
        
        m_storage.reset();
        assertEquals(DEFAULT_VALUE, m_storage.getValue(m_andorra));
    }
    

    public void testRemove() throws Exception {
        // remove should reset the setting to default value
        String newValue = "99.999";
        m_storage.setValue(m_andorra, newValue);
        assertEquals(newValue, m_storage.getValue(m_andorra));
        m_storage.flush();
        m_storage.revertSettingToDefault(m_andorra);
        m_storage.flush();

        BufferedReader reader = new BufferedReader(new FileReader(new File(m_configDirectory,
                CONFIG_FILE)));

        boolean found = false;
        for (String line = reader.readLine(); line != null; line = reader.readLine()) {
            if (line.startsWith(m_andorra.getName())) {
                assertTrue(line.endsWith(DEFAULT_VALUE));
                found = true;
            }
        }
        assertTrue("Setting not found", found);
    }
    
    public void testSetNull() throws Exception {
        m_storage.setValue(m_andorra, null);
    }

}
