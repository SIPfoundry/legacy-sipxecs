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
import java.util.Arrays;
import java.util.Iterator;
import java.util.Map;

import org.dbunit.dataset.IDataSet;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.test.IntegrationTestCase;
import org.sipfoundry.sipxconfig.test.TestHelper;
import org.springframework.orm.hibernate3.HibernateObjectRetrievalFailureException;

public class UploadTestIntegration extends IntegrationTestCase {
    private UploadManager m_uploadManager;

    final static UploadSpecification UNMANAGED = new UploadSpecification("upload",
    "unmanagedUpload");
    static {
        UNMANAGED.setModelFilePath("unmanagedPhone/upload.xml");
    }

    @Override
    protected void onSetUpBeforeTransaction() throws Exception {
        super.onSetUpBeforeTransaction();
        clear();
    }

    public void testLoadSettings() throws Exception {
        Upload f = m_uploadManager.newUpload(UploadTest.UNMANAGED);
        f.getSettings();
    }

    public void testSave() throws Exception {
        Upload f = m_uploadManager.newUpload(UploadTest.UNMANAGED);
        f.setName("bezerk");
        m_uploadManager.saveUpload(f);
        commit();

        Map<String, Object> actual = db().queryForMap("select * from upload");
        assertEquals("bezerk", actual.get("name"));
        assertEquals(false, actual.get("deployed"));
        assertEquals("unmanagedUpload", actual.get("specification_id"));
    }

    public void testLoadAndDelete() throws Exception {
        loadDataSet("upload/UploadSeed.db.xml");
        Upload f = m_uploadManager.loadUpload(new Integer(1000));
        assertEquals("test upload", f.getName());
        assertEquals(UploadTest.UNMANAGED.getSpecificationId(), f.getSpecification().getSpecificationId());

        Integer id = f.getId();
        m_uploadManager.deleteUpload(f);
        try {
            m_uploadManager.loadUpload(id);
            fail();
        } catch (HibernateObjectRetrievalFailureException x) {
            assertTrue(true);
        }

        IDataSet actual = TestHelper.getConnection().createDataSet();
        assertEquals(0, actual.getTable("Upload").getRowCount());
    }

    public void testUndeploy() throws Exception {
        loadDataSet("upload/GetUploadSeed.db.xml");
        Upload upload = m_uploadManager.getUpload().iterator().next();
        m_uploadManager.undeploy(upload);
        Upload fresh = m_uploadManager.getUpload().iterator().next();
        assertFalse(fresh.isDeployed());
    }

    public void testGetUpload() throws Exception {
        loadDataSet("upload/GetUploadSeed.db.xml");
        Upload[] f = m_uploadManager.getUpload().toArray(new Upload[0]);
        assertEquals(2, f.length);
        assertEquals("harriot", f[0].getName());
        assertEquals("ozzie", f[1].getName());
    }

    public void testMoreThanOneUnmanagedActive() throws Exception {
        loadDataSet("upload/GetUploadSeed.db.xml");
        Upload upload = m_uploadManager.newUpload(UploadTest.UNMANAGED);
        upload.setName("monk parakeet");

        m_uploadManager.deploy(upload);
        assertTrue(upload.isDeployed());
    }

    public void testRestrictDuplicateUploadTypes() throws Exception {
        loadDataSet("upload/GetUploadSeed.db.xml");
        Upload[] existing = m_uploadManager.getUpload().toArray(new Upload[0]);
        Upload upload = m_uploadManager.newUpload(UploadTest.UNMANAGED);

        // switch specification to managed so that we can test restrictions
        UploadSpecification us = new UploadSpecification("test", "unmanagedUpload");
        us.setManaged(true);
        us.setModelDir(upload.getSpecification().getModelDir());
        us.setModelFilePath(upload.getSpecification().getModelFilePath());
        upload.setSpecification(us);

        upload.setName("monk parakeet");

        try {
            m_uploadManager.deploy(upload);
            fail();
        } catch (UserException expected) {
            assertFalse(upload.isDeployed());
        }

        m_uploadManager.undeploy(existing[0]);
        try {
            m_uploadManager.deploy(upload);
            fail();
        } catch (UserException expected) {
            assertFalse(upload.isDeployed());
        }

        m_uploadManager.undeploy(existing[1]);
        m_uploadManager.deploy(upload);
        assertTrue(upload.isDeployed());
    }

    public void testRestrictDuplicateUploadName() throws Exception {
        loadDataSet("upload/GetUploadSeed.db.xml");
        Upload upload = m_uploadManager.newUpload(UploadTest.UNMANAGED);
        // set upload name to be same as the existing upload defined in GetUploadSeed.db.xml
        upload.setName("harriot");

        try {
            m_uploadManager.saveUpload(upload);
            fail();
        } catch (UserException expected) {
            assertTrue(upload.isNew());
        }
    }

    public void testLoadSubclass() throws Exception {
        loadDataSet("upload/ZipUploadSeed.db.xml");
        Iterator<Upload> existing = m_uploadManager.getUpload().iterator();
        assertNotNull(existing.next());
        assertFalse(existing.hasNext());
    }

    public void setUploadManager(UploadManager uploadManager) {
        m_uploadManager = uploadManager;
    }

    private String mkdirs(String dir) {
        new File(dir).mkdirs();
        return dir;
    }

    public void testMissingUploads() throws Exception {
        Upload upload1 = new Upload(UNMANAGED);
        Upload upload2 = new Upload(UNMANAGED);

        upload1.setDirectoryId(String.valueOf(System.currentTimeMillis()));
        upload1.setModelFilesContext(TestHelper.getModelFilesContext());
        upload1.setUploadRootDirectory(TestHelper.getTestDirectory() + "/upload1");
        upload1.setName("Upload1");
        upload2.setName("Upload2");
        upload2.setDirectoryId(String.valueOf(System.currentTimeMillis()));
        upload2.setModelFilesContext(TestHelper.getModelFilesContext());
        upload2.setUploadRootDirectory(TestHelper.getTestDirectory() + "/upload2");

        File dir1 = new File(mkdirs(upload1.getUploadDirectory()));
        File file11 = File.createTempFile("upload-test", ".dat", dir1);
        File file12 = File.createTempFile("upload-test2", ".dat", dir1);
        File file13 = File.createTempFile("upload-test3", ".dat", dir1);
        upload1.setSettingValue("files/file1", file11.getName());
        upload1.setSettingValue("files/file2", file12.getName());
        upload1.setSettingValue("files/file3", file13.getName());

        File dir2 = new File(mkdirs(upload1.getUploadDirectory()));
        File file21 = File.createTempFile("upload-2test2", ".dat", dir2);
        File file22 = File.createTempFile("upload-2test22", ".dat", dir2);
        upload2.setSettingValue("files/file1", file21.getName());
        upload2.setSettingValue("files/file2", file22.getName());

        assertTrue(file11.exists());
        assertTrue(file12.exists());
        assertTrue(file13.exists());
        assertTrue(file21.exists());
        assertTrue(file22.exists());

        file11.delete();
        file13.delete();
        file21.delete();
        file22.delete();
        assertFalse(file11.exists());
        assertTrue(file12.exists());
        assertFalse(file13.exists());
        assertFalse(file21.exists());
        assertFalse(file21.exists());

        assertEquals(upload1.getSettingValue("files/file1"), file11.getName());
        assertEquals(upload1.getSettingValue("files/file2"), file12.getName());
        assertEquals(upload1.getSettingValue("files/file3"), file13.getName());
        assertEquals(upload2.getSettingValue("files/file1"), file21.getName());
        assertEquals(upload2.getSettingValue("files/file2"), file22.getName());

        m_uploadManager.clearMissingUploads(Arrays.asList(upload1, upload2));

        assertNull(upload1.getSettingValue("files/file1"));
        assertEquals(upload1.getSettingValue("files/file2"), file12.getName());
        assertNull(upload1.getSettingValue("files/file3"));
        assertNull(upload2.getSettingValue("files/file1"));
        assertNull(upload2.getSettingValue("files/file2"));
    }
}
