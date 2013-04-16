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

import java.io.File;
import java.io.FileFilter;
import java.io.IOException;

import org.apache.commons.io.FileUtils;
import org.apache.commons.io.filefilter.WildcardFileFilter;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.upload.Upload;

public class PolycomUpload extends Upload {
    private static final Log LOG = LogFactory.getLog(PolycomUpload.class);
    private static final String POLYCOM_DIR = "/polycom/";
    private static final String VERSION = "firmware/version";
    private static final String SIP_APP = "*.sip.ld";
    private String m_profileDir;

    public String getProfileDir() {
        return m_profileDir;
    }

    public void setProfileDir(String profileDir) {
        m_profileDir = profileDir;
    }

    @Override
    public void deploy() {
        String destination = new StringBuilder(getDestinationDirectory()).append(POLYCOM_DIR)
                .append(getSettingValue(VERSION)).toString();
        File destinationFolder = new File(destination);
        super.setDestinationDirectory(destination);
        super.deploy();
        // we need to make sure however is packed polycom fw (*.sip.ld)will be in
        // polycom/{VERSION}
        FileFilter fileFilter = new WildcardFileFilter(SIP_APP);
        File[] sipApp = destinationFolder.listFiles(fileFilter);
        if (sipApp.length != 0) {
            return;
        }
        // This means it is packed in another folder (thanks Polycom)
        // so we need to cd to that folder and move contents to ../d
        File[] folder = destinationFolder.listFiles();
        File fwFolder = null;
        for (int i = 0; i < folder.length; i++) {
            if (folder[i].isFile()) {
                continue;
            }
            if (folder[i].listFiles(fileFilter).length == 0) {
                continue;
            }
            fwFolder = folder[i];
        }
        if (fwFolder == null) {
            LOG.warn("Cannot find sip.ld.");
            return;
        }
        File[] fwFiles = fwFolder.listFiles();
        // cd into folder and move all files to destination
        try {
            for (int i = 0; i < fwFiles.length; i++) {
                if (fwFiles[i].isDirectory()) {
                    if (new File(destinationFolder, fwFiles[i].getName()).exists()) {
                        FileUtils.moveDirectoryToDirectory(fwFiles[i],
                                new File(destinationFolder, fwFiles[i].getName()), true);
                    } else {
                        FileUtils.moveDirectoryToDirectory(fwFiles[i], destinationFolder, false);
                    }
                } else if (fwFiles[i].isFile()) {
                    FileUtils.moveFileToDirectory(fwFiles[i], destinationFolder, false);
                }
            }
        } catch (IOException e) {
            LOG.error("IOException while moving files. Please check destination folder.", e);
        }
    }

    @Override
    public void undeploy() {
        super.setDestinationDirectory(getDestinationDirectory() + POLYCOM_DIR + getSettingValue(VERSION));
        super.undeploy();
        File spipLoc = new File(getDestinationDirectory() + "/SoundPointIPLocalization");
        try {
            if (spipLoc.exists()) {
                FileUtils.deleteDirectory(spipLoc);
            }
            File config = new File(getDestinationDirectory() + "/Config");
            if (config.exists()) {
                FileUtils.deleteDirectory(config);
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
