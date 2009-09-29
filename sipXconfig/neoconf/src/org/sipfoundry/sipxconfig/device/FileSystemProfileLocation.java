/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.device;

import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;

import org.apache.commons.io.FileUtils;
import org.apache.commons.io.FilenameUtils;
import org.apache.commons.io.IOUtils;
import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

public class FileSystemProfileLocation implements ProfileLocation {
    static final Log LOG = LogFactory.getLog(ProfileLocation.class);

    private String m_parentDir;

    public void setParentDir(String parentDir) {
        m_parentDir = parentDir;
    }

    public OutputStream getOutput(String profileName) {
        File profileFile = getProfileFile(profileName);
        ProfileUtils.makeParentDirectory(profileFile);
        try {
            return new BufferedOutputStream(new FileOutputStream(profileFile));
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
    }

    public void removeProfile(String profileName) {
        if (StringUtils.isEmpty(profileName)) {
            return;
        }
        try {
            File file = getProfileFile(profileName);
            FileUtils.forceDelete(file);
        } catch (IOException e) {
            // ignore delete failure
            LOG.info(e.getMessage());
        }
    }

    private File getProfileFile(String profileName) {
        String profileFilename = FilenameUtils.concat(m_parentDir, profileName);
        File profileFile = new File(profileFilename);
        return profileFile;
    }

    public void closeOutput(OutputStream stream) {
        IOUtils.closeQuietly(stream);
    }
}
