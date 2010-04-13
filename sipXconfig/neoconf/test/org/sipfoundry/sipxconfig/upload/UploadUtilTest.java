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

public class UploadUtilTest extends TestCase {
    public void testSetUploads() {
        Upload upload = new Upload();
        upload.setUploadRootDirectory("uploaddir");
        upload.setDirectoryId("dirId");
        UploadSpecification specification = new UploadSpecification();
        upload.setSettings(TestHelper.loadSettings("acmePhone/upload.xml"));
        upload.setSpecification(specification);
        String[] args = new String[] { "bean-id", "upload-id", "files/file1", "xyz.dat"};

        IMocksControl uploadManagerControl = EasyMock.createControl();
        UploadManager uploadManager = uploadManagerControl.createMock(UploadManager.class);
        uploadManager.saveUpload(upload);
        uploadManagerControl.replay();

        UploadUtil task = new UploadUtil() {
            @Override
            void copyFile(File f, File destination) {
                assertEquals("xyz.dat", f.getPath());
                assertEquals("uploaddir/dirId/xyz.dat", destination.getPath());
            }
        };

        task.setUploadManager(uploadManager);
        task.setUploads(upload, args);

        uploadManagerControl.verify();
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
        uploadManager.newUpload(spec);
        uploadManagerControl.andReturn(upload);
        uploadManager.saveUpload(upload);
        uploadManagerControl.replay();

        UploadUtil task = new UploadUtil();
        task.setUploadSpecificationSource(uploadTypes);
        task.setUploadManager(uploadManager);

        String[] args = new String[] { "bean-id", "upload-id"};

        Upload u = task.addUpload(args);
        assertNotNull(u);

        uploadTypesControl.verify();
        uploadManagerControl.verify();
    }

}

