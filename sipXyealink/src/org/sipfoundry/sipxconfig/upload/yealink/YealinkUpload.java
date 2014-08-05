/*
 * Copyright (c) 2013 SibTelCom, JSC (SIPLABS Communications). All rights reserved.
 * Contributed to SIPfoundry and eZuce, Inc. under a Contributor Agreement.
 *
 * Developed by Konstantin S. Vishnivetsky
 *
 * This library or application is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Affero General Public License (AGPL) as published by the Free
 * Software Foundation; either version 3 of the License, or (at your option) any later version.
 *
 * This library or application is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License (AGPL) for
 * more details.
 *
 */

package org.sipfoundry.sipxconfig.upload.yealink;

import java.io.File;
import java.io.IOException;

import org.apache.commons.io.FileUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.upload.Upload;
import org.sipfoundry.sipxconfig.upload.UploadSpecification;

public class YealinkUpload extends Upload {
    public static final String DIR_YEALINK = "/yealink";
    public static final String DIR_RINGTONES = "/RingTones";
    public static final String DIR_WALLPAPERS = "/WallPapers";
    public static final String DIR_SCREENSAVERS = "/ScreenSavers";
    public static final String DIR_LANGUAGES = "/Languages";
    private static final Log LOG = LogFactory.getLog(YealinkUpload.class);

    public YealinkUpload() {
    }

    protected YealinkUpload(String beanId) {
        super(beanId);
    }

    public YealinkUpload(UploadSpecification specification) {
        super(specification);
    }

    @Override
    public void deploy() {
        super.setDestinationDirectory(getDestinationDirectory() + DIR_YEALINK);
        super.deploy();
    }

    @Override
    public void undeploy() {
        super.setDestinationDirectory(getDestinationDirectory() + DIR_YEALINK);
        super.undeploy();
        try {
            File mainLoc = new File(getDestinationDirectory());
            if (mainLoc.exists()) {
                FileUtils.deleteDirectory(mainLoc);
            }
            File rtLoc = new File(getDestinationDirectory() + DIR_RINGTONES);
            if (rtLoc.exists()) {
                FileUtils.deleteDirectory(rtLoc);
            }
            File wpLoc = new File(getDestinationDirectory() + DIR_WALLPAPERS);
            if (wpLoc.exists()) {
                FileUtils.deleteDirectory(wpLoc);
            }
            File ssLoc = new File(getDestinationDirectory() + DIR_SCREENSAVERS);
            if (ssLoc.exists()) {
                FileUtils.deleteDirectory(ssLoc);
            }
            File lngLoc = new File(getDestinationDirectory() + DIR_LANGUAGES);
            if (lngLoc.exists()) {
                FileUtils.deleteDirectory(lngLoc);
            }
        } catch (IOException e) {
            LOG.error("IOException while deleting folder.", e);
        }
    }

    @Override
    public FileRemover createFileRemover() {
        return new FileRemover();
    }

    public class FileRemover extends Upload.FileRemover {
        @Override
        public void removeFile(File dir, String name) {
            File victim = new File(dir, name);
            if (!victim.exists()) {
                String[] splits = name.split("/");
                if (splits.length >= 2) {
                    victim = new File(dir, splits[1]);
                }
            }
            victim.delete();
        }
    }
}
