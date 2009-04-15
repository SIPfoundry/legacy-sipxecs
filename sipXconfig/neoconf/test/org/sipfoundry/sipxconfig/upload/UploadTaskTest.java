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

import junit.framework.TestCase;

import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.device.ModelSource;

public class UploadTaskTest extends TestCase {
    private UploadTask m_task;
    
    protected void setUp() {
        m_task = new UploadTask();
    }
    
    public void testSetUploads() {
        IMocksControl copyUtilControl = org.easymock.classextension.EasyMock.createControl();
        UploadTask copyUtil = copyUtilControl.createMock(UploadTask.class);
        copyUtil.copyFile(new File("xyz.dat"), new File("uploaddir/dirId/xyz.dat"));
        copyUtilControl.replay();

        Upload upload = new Upload();
        upload.setUploadRootDirectory("uploaddir");
        upload.setDirectoryId("dirId");
        UploadSpecification specification = new UploadSpecification();
        upload.setSettings(TestHelper.loadSettings("acmePhone/upload.xml"));
        upload.setSpecification(specification);
        String[] args = new String[] { "bean-id", "upload-id", "files/file1", "xyz.dat"};
        m_task.setUploads(copyUtil, upload, args);
       
        copyUtilControl.verify();
    }

    public void testAddUpload() {
        Upload upload = new Upload();
        UploadSpecification spec = new UploadSpecification();
        
        IMocksControl uploadTypesControl = EasyMock.createControl();
        ModelSource<UploadSpecification> uploadTypes = uploadTypesControl.createMock(ModelSource.class);
        uploadTypes.getModel("upload-id");
        uploadTypesControl.andReturn(spec);
        uploadTypesControl.replay();
        
        IMocksControl uploadManagerControl = EasyMock.createControl();
        UploadManager uploadManager = uploadManagerControl.createMock(UploadManager.class);
        uploadManager.isActiveUploadById(spec);
        uploadManagerControl.andReturn(false);
        uploadManager.newUpload(spec);
        uploadManagerControl.andReturn(upload);
        uploadManager.saveUpload(upload);
        uploadManagerControl.replay();
        
        m_task.setUploadSpecificationSource(uploadTypes);
        m_task.setUploadManager(uploadManager);
        
        String[] args = new String[] { "bean-id", "upload-id"};
        
        Upload u = m_task.addUpload(args);
        assertNotNull(u);
       
        uploadTypesControl.verify();
        uploadManagerControl.verify();
    }

}

