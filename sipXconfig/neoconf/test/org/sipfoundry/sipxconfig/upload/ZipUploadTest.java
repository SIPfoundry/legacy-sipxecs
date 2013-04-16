/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
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
import org.sipfoundry.sipxconfig.setting.type.FileSetting;
import org.sipfoundry.sipxconfig.test.TestHelper;

public class ZipUploadTest extends TestCase {
    private File m_expandDir;
    private File m_zipFile;

    @Override
    protected void setUp() throws IOException {
        m_zipFile = TestHelper.getResourceAsFile(getClass(), "zip-test.zip");
        m_expandDir = TestHelper.createTempDir("zip-test");
    }

    @Override
    protected void tearDown() throws Exception {
        FileUtils.deleteDirectory(m_expandDir);
    }

    public void testDeployUndeploy() throws Exception {
        Upload.deployZipFile(m_expandDir, m_zipFile, new FileSetting());
        File file1 = new File(m_expandDir, "zip-test/subdir/file1.txt");
        assertTrue(file1.exists());
        assertTrue(IOUtils.contentEquals(getClass().getResourceAsStream("file1.txt"), new FileInputStream(file1)));
        File file3 = new File(m_expandDir, "zip-test/file3.bin");
        assertTrue(file3.exists());
        InputStream stream3 = getClass().getResourceAsStream("file3.bin");
        assertTrue(IOUtils.contentEquals(stream3, new FileInputStream(file3)));
        Upload.undeployZipFile(m_expandDir, m_zipFile, new FileSetting());
        assertFalse(file1.exists());
    }

    public void testDeployUndeployExcludes() throws Exception {
        FileSetting type = new FileSetting();

        // This should do nothing, since it doesn't match any paths.
        type.addZipExclude("zip");

        // An individual file excluded.
        type.addZipExclude("zip-test/file3.bin");

        // An entire directory excluded.
        type.addZipExclude("zip-test/subdir"); // Intentionally no trailing separator.

        // All the files and directories from the zip archive.
        File dir1 = new File(m_expandDir, "zip-test");
        File dir2 = new File(m_expandDir, "zip-test/subdir");
        File file1 = new File(m_expandDir, "zip-test/subdir/file1.txt");
        File file2 = new File(m_expandDir, "zip-test/file2.txt");
        File file3 = new File(m_expandDir, "zip-test/file3.bin");

        // Some should exist, and some should not.
        Upload.deployZipFile(m_expandDir, m_zipFile, type);
        assertTrue(dir1.exists());
        assertTrue(!dir2.exists());
        assertTrue(!file1.exists());
        assertTrue(file2.exists());
        assertTrue(!file3.exists());

        // Now manually create this file that was excluded, to show that an undeploy
        // will not destroy it.
        FileUtils.touch(file3);

        // Some should exist, and some should not.
        Upload.undeployZipFile(m_expandDir, m_zipFile, type);
        assertTrue(dir1.exists()); // do not clean up directory, no guarantee we created them
        assertTrue(!dir2.exists());
        assertTrue(!file1.exists());
        assertTrue(!file2.exists());
        assertTrue(file3.exists());
    }

    public void testUndeployMissing() throws Exception {
        File file = new File("test_missing_file.zip");
        assertFalse(file.exists());
        Upload.undeployZipFile(m_expandDir, file, new FileSetting());
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
