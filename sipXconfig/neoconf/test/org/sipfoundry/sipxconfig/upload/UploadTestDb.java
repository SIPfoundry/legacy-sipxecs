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

import java.util.Iterator;

import org.dbunit.Assertion;
import org.dbunit.dataset.IDataSet;
import org.dbunit.dataset.ITable;
import org.dbunit.dataset.ReplacementDataSet;
import org.sipfoundry.sipxconfig.SipxDatabaseTestCase;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.common.UserException;
import org.springframework.orm.hibernate3.HibernateObjectRetrievalFailureException;

public class UploadTestDb extends SipxDatabaseTestCase {
    private UploadManager m_manager;

    @Override
    protected void setUp() throws Exception {
        m_manager = (UploadManager) TestHelper.getApplicationContext().getBean(UploadManager.CONTEXT_BEAN_NAME);
    }

    public void testLoadSettings() throws Exception {
        Upload f = m_manager.newUpload(UploadTest.UNMANAGED);
        f.getSettings();
    }

    public void testSave() throws Exception {
        TestHelper.cleanInsert("ClearDb.xml");
        Upload f = m_manager.newUpload(UploadTest.UNMANAGED);
        f.setName("bezerk");
        m_manager.saveUpload(f);

        IDataSet expectedDs = TestHelper.loadDataSetFlat("upload/SaveUploadExpected.db.xml");
        ReplacementDataSet expectedRds = new ReplacementDataSet(expectedDs);
        expectedRds.addReplacementObject("[null]", null);
        expectedRds.addReplacementObject("[upload_id]", f.getPrimaryKey());
        ITable expected = expectedRds.getTable("Upload");
        ITable actual = TestHelper.getConnection().createDataSet().getTable("Upload");

        Assertion.assertEquals(expected, actual);
    }

    public void testLoadAndDelete() throws Exception {
        TestHelper.cleanInsert("ClearDb.xml");
        TestHelper.cleanInsertFlat("upload/UploadSeed.db.xml");
        Upload f = m_manager.loadUpload(new Integer(1000));
        assertEquals("test upload", f.getName());
        assertEquals(UploadTest.UNMANAGED.getSpecificationId(), f.getSpecification().getSpecificationId());

        Integer id = f.getId();
        m_manager.deleteUpload(f);
        try {
            m_manager.loadUpload(id);
            fail();
        } catch (HibernateObjectRetrievalFailureException x) {
            assertTrue(true);
        }

        IDataSet actual = TestHelper.getConnection().createDataSet();
        assertEquals(0, actual.getTable("Upload").getRowCount());
    }

    public void testUndeploy() throws Exception {
        TestHelper.cleanInsert("ClearDb.xml");
        TestHelper.cleanInsertFlat("upload/GetUploadSeed.db.xml");
        Upload upload = m_manager.getUpload().iterator().next();
        m_manager.undeploy(upload);
        Upload fresh = m_manager.getUpload().iterator().next();
        assertFalse(fresh.isDeployed());
    }

    public void testGetUpload() throws Exception {
        TestHelper.cleanInsert("ClearDb.xml");
        TestHelper.cleanInsertFlat("upload/GetUploadSeed.db.xml");
        Upload[] f = m_manager.getUpload().toArray(new Upload[0]);
        assertEquals(2, f.length);
        assertEquals("harriot", f[0].getName());
        assertEquals("ozzie", f[1].getName());
    }

    public void testMoreThanOneUnmanagedActive() throws Exception {
        TestHelper.cleanInsert("ClearDb.xml");
        TestHelper.cleanInsertFlat("upload/GetUploadSeed.db.xml");
        Upload upload = m_manager.newUpload(UploadTest.UNMANAGED);
        upload.setName("monk parakeet");

        m_manager.deploy(upload);
        assertTrue(upload.isDeployed());
    }

    public void testRestrictDuplicateUploadTypes() throws Exception {
        TestHelper.cleanInsert("ClearDb.xml");
        TestHelper.cleanInsertFlat("upload/GetUploadSeed.db.xml");
        Upload[] existing = m_manager.getUpload().toArray(new Upload[0]);
        Upload upload = m_manager.newUpload(UploadTest.UNMANAGED);

        // switch specification to managed so that we can test restrictions
        UploadSpecification us = new UploadSpecification("test", "unmanagedUpload");
        us.setManaged(true);
        us.setModelDir(upload.getSpecification().getModelDir());
        us.setModelFilePath(upload.getSpecification().getModelFilePath());
        upload.setSpecification(us);

        upload.setName("monk parakeet");

        try {
            m_manager.deploy(upload);
            fail();
        } catch (UserException expected) {
            assertFalse(upload.isDeployed());
        }

        m_manager.undeploy(existing[0]);
        try {
            m_manager.deploy(upload);
            fail();
        } catch (UserException expected) {
            assertFalse(upload.isDeployed());
        }

        m_manager.undeploy(existing[1]);
        m_manager.deploy(upload);
        assertTrue(upload.isDeployed());
    }

    public void testRestrictDuplicateUploadName() throws Exception {
        TestHelper.cleanInsert("ClearDb.xml");
        TestHelper.cleanInsertFlat("upload/GetUploadSeed.db.xml");
        Upload upload = m_manager.newUpload(UploadTest.UNMANAGED);
        // set upload name to be same as the existing upload defined in GetUploadSeed.db.xml
        upload.setName("harriot");

        try {
            m_manager.saveUpload(upload);
            fail();
        } catch (UserException expected) {
            assertTrue(upload.isNew());
        }
    }

    public void testLoadSubclass() throws Exception {
        TestHelper.cleanInsertFlat("upload/ZipUploadSeed.db.xml");
        Iterator<Upload> existing = m_manager.getUpload().iterator();
        assertNotNull(existing.next());
        assertFalse(existing.hasNext());
    }
}
