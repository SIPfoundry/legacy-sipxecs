/**
 *
 *
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.phone.polycom;

import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.setup.SetupListener;
import org.sipfoundry.sipxconfig.setup.SetupManager;
import org.sipfoundry.sipxconfig.upload.Upload;
import org.sipfoundry.sipxconfig.upload.UploadManager;

/**
 * Task for deleting the device files in 4.6 after upgrade to 4.6.1
 */
public class DeviceFilesCleaner implements SetupListener {
    private static final Log LOG = LogFactory.getLog(DeviceFilesCleaner.class);
    private static final String DEVICE_CLEANER_FLAG = "upgrade-4.6-4.7-device-files-cleaner";
    private UploadManager m_uploadManager;

    @Override
    public boolean setup(SetupManager manager) {
        if (manager.isFalse(DEVICE_CLEANER_FLAG)) {
            for (Upload upload : m_uploadManager.getUpload()) {
                if (StringUtils.equals(upload.getSpecificationId(), "polycomFirmware")) {
                    try {
                        m_uploadManager.deleteUpload(upload);
                    } catch (Exception e) {
                        LOG.warn(String.format("An error was thrown when trying to remove a device file: %s. "
                                + "Error message is: %s. Turn on debug to see the error stacktrace.",
                                upload.getName(), e.getMessage()));
                        LOG.debug("Device file delete error", e);
                    }
                }
            }
        }
        manager.setTrue(DEVICE_CLEANER_FLAG);
        return true;
    }

    public void setUploadManager(UploadManager uploadManager) {
        m_uploadManager = uploadManager;
    }

}
