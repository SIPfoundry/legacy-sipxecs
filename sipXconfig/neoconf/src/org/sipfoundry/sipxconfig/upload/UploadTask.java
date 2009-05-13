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

import org.sipfoundry.sipxconfig.common.SystemTaskEntryPoint;

/**
 * "upload" from CLI, useful for OEM situations where you'd like to have system ship with
 * default firmware installed.
 */
public class UploadTask implements SystemTaskEntryPoint {
    private UploadUtil m_uploadUtil;

    public void setUploadUtil(UploadUtil uploadUtil) {
        m_uploadUtil = uploadUtil;
    }

    /**
     * @params  uploadId, settingName0, fileName0, settingName1, fileName1, ...
     */
    public void runSystemTask(String[] args) {
        Upload upload = m_uploadUtil.addUpload(args);
        m_uploadUtil.setUploads(upload, args);
    }
}
