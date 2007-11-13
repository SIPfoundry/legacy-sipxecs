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

import java.io.File;
import java.io.InputStream;
import java.io.StringWriter;
import java.io.Writer;
import java.util.Properties;

import junit.framework.TestCase;

import org.apache.commons.io.FileUtils;
import org.apache.commons.io.IOUtils;
import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.test.TestUtil;

public class ConfigFileWriterTest extends TestCase {
    private String m_originalContent;
    private String m_configDirectory;
    private Properties m_properties;
    private File m_destinationFile;

    protected void setUp() throws Exception {
        InputStream configStream = ConfigFileStorageTest.class
                .getResourceAsStream(ConfigFileStorageTest.CONFIG_FILE);
        m_configDirectory = TestHelper.getTestDirectory();
        TestHelper.copyStreamToDirectory(configStream, m_configDirectory,
                ConfigFileStorageTest.CONFIG_FILE);

        // open stream again
        configStream = ConfigFileStorageTest.class
                .getResourceAsStream(ConfigFileStorageTest.CONFIG_FILE);
        configStream.mark(-1);
        m_properties = new Properties();
        m_properties.load(configStream);

        m_destinationFile = new File(m_configDirectory, ConfigFileStorageTest.CONFIG_FILE);
        configStream.reset();
        Writer buffer = new StringWriter();
        IOUtils.copy(configStream, buffer);

        m_originalContent = buffer.toString();

        IOUtils.closeQuietly(configStream);
    }

    public void testWriteAndReset() throws Exception {
        // change existing property
        m_properties.setProperty("Libya", "100");
        // add a new one
        m_properties.setProperty("Kenya", "55");
        ConfigFileWriter writer = new ConfigFileWriter(m_destinationFile);
        writer.store(m_properties);

        String newContent = FileUtils.readFileToString(m_destinationFile, "8859_1");        

        // files need to be quite similar - instead of checking it line by line just
        // make sure that the first difference is related to Libya's change
        String diff = StringUtils.difference(m_originalContent, newContent);        
        assertTrue(diff.startsWith(" 100"));
        assertTrue(newContent.endsWith(TestUtil.cleanEndOfLines("Kenya : 55\n")));
        
        writer.reset();
        String emptyContent = FileUtils.readFileToString(m_destinationFile, "8859_1");
        assertTrue(StringUtils.isEmpty(emptyContent));
    }
}
