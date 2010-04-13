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
import java.io.IOException;

import org.apache.commons.io.FileUtils;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.device.ModelSource;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.type.FileSetting;
import org.springframework.beans.factory.annotation.Required;

public class UploadUtil {
    private UploadManager m_uploadManager;
    private ModelSource<UploadSpecification> m_uploadSpecSource;

    /**
     * @params uploadId, settingName0, fileName0, settingName1, fileName1, ...
     */

    public Upload addUpload(String[] args) {
        return addUpload(args, "System Default");
    }

    public synchronized Upload addUpload(String[] args, String name) {
        String uploadSpecId = args[1];
        UploadSpecification spec = m_uploadSpecSource.getModel(uploadSpecId);
        if (spec == null) {
            illegalArgument("No such upload type '%s'", uploadSpecId);
        }

        String uploadName = name.concat(" - " + spec.getLabel());

        Upload upload = m_uploadManager.newUpload(spec);
        upload.setName(uploadName);
        upload.setDescription("Default System Firmware");
        m_uploadManager.saveUpload(upload);

        return upload;
    }

    public synchronized void setUploads(Upload upload, String[] args) {
        for (int i = 2; i < args.length; i = i + 2) {
            String settingName = args[i];
            String fileName = args[i + 1];
            Setting s = upload.getSettings().getSetting(settingName);
            if (s == null) {
                illegalArgument("No such setting '%s'", settingName);
            }
            FileSetting type = (FileSetting) s.getType();
            File in = new File(fileName);
            File uploadDir = new File(upload.getUploadDirectory());
            File uploadFile = new File(uploadDir, in.getName());
            copyFile(in, uploadFile);
            type.setDirectory(uploadDir.getPath());
            s.setValue(uploadFile.getName());
        }
        m_uploadManager.saveUpload(upload);
    }

    public synchronized void deploy(Upload upload) {
        if (!m_uploadManager.isActiveUploadById(upload.getSpecification())) {
            m_uploadManager.deploy(upload);
        }
    }

    public synchronized void forceDeploy(Upload upload) {
        m_uploadManager.undeploy(upload.getSpecification());
        m_uploadManager.deploy(upload);
    }

    private void illegalArgument(String template, Object... params) {
        throw new UserException(String.format(template, params));
    }

    void copyFile(File f, File destination) {
        try {
            FileUtils.copyFile(f, destination);
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
    }

    @Required
    public void setUploadManager(UploadManager uploadManager) {
        m_uploadManager = uploadManager;
    }

    @Required
    public void setUploadSpecificationSource(ModelSource<UploadSpecification> uploadSpecSource) {
        m_uploadSpecSource = uploadSpecSource;
    }
}
