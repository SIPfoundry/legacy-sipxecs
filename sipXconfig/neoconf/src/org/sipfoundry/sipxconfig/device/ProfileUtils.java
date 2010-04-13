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

import java.io.File;
import java.io.IOException;

import org.apache.commons.io.FileUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

public final class ProfileUtils {
    static final Log LOG = LogFactory.getLog(ProfileUtils.class);

    private ProfileUtils() {
        // Utility class - do not instantiate
    }

    public static void removeProfileFiles(File[] profileFiles) {
        for (int i = 0; i < profileFiles.length; i++) {
            try {
                FileUtils.forceDelete(profileFiles[i]);
            } catch (IOException e) {
                // ignore delete failure
                LOG.info(e.getMessage());
            }
        }
    }

    public static void makeParentDirectory(File f) {
        File parentDir = f.getParentFile();
        parentDir.mkdirs();
        if (!parentDir.exists()) {
            throw new RuntimeException("Could not create parent directory for file "
                    + f.getPath());
        }
    }

}
