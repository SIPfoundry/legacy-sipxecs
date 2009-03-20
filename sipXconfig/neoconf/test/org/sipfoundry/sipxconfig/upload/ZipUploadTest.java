/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.upload;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;

import junit.framework.TestCase;

import org.apache.commons.io.FileUtils;
import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.test.TestUtil;

public class ZipUploadTest extends TestCase {
    private File m_expandDir;
    private File m_zipFile;

    protected void setUp() throws IOException {
        File thisDir = new File(TestUtil.getTestSourceDirectory(getClass()));
        m_zipFile = new File(thisDir, "zip-test.zip");
        m_expandDir = TestUtil.createTempDir("zip-test");
    }

    protected void tearDown() throws Exception {
        FileUtils.deleteDirectory(m_expandDir);
    }

    public void testDeployUndeploy() throws Exception {
        Upload.deployZipFile(m_expandDir, m_zipFile);
        File file1 = new File(m_expandDir, "zip-test/subdir/file1.txt");
        assertTrue(file1.exists());
        assertTrue(IOUtils.contentEquals(getClass().getResourceAsStream("file1.txt"),
                new FileInputStream(file1)));
        File file3 = new File(m_expandDir, "zip-test/file3.bin");
        assertTrue(file3.exists());
        InputStream stream3 = getClass().getResourceAsStream("file3.bin");
        assertTrue(IOUtils.contentEquals(stream3, new FileInputStream(file3)));
        Upload.undeployZipFile(m_expandDir, m_zipFile);
        assertFalse(file1.exists());
    }

    public void testUndeployMissing() throws Exception {
        File file = new File("test_missing_file.zip");
        assertFalse(file.exists());
        Upload.undeployZipFile(m_expandDir, file);
    }

    public void testFile() throws Exception {
        File file = File.createTempFile("file3", "bin");
        FileOutputStream stream = new FileOutputStream(file);
        for (int i = 100; i < 400; i++) {
            stream.write(i);
        }
        stream.close();
    }
}
