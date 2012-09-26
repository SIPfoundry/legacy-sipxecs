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
package org.sipfoundry.sipxconfig.common;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;

import org.apache.commons.io.FileUtils;
import org.apache.commons.io.IOUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

/**
 * Release information
 */
public class ReleaseInfo {
    private static final Log LOG = LogFactory.getLog(ReleaseInfo.class);
    private File m_packageInfoExec;
    private File m_releaseInfoFile;

    public void setPackageInfoExec(String packageInfoExec) {
        m_packageInfoExec = new File(packageInfoExec);
    }

    public void setReleaseInfoFile(String releaseInfoFile) {
        m_releaseInfoFile = new File(releaseInfoFile);
    }

    File getReleaseInfoFile() {
        return m_releaseInfoFile;
    }

    public String getReleaseUpdate() {
        try {
            return FileUtils.readFileToString(m_releaseInfoFile);
        } catch (IOException e) {
            return "unknown";
        }
    }

    public String getPackageInfo() {
        if (m_packageInfoExec.exists()) {
            InputStream output = null;
            try {
                Process p = Runtime.getRuntime().exec(m_packageInfoExec.getPath());
                output = p.getInputStream();
                return IOUtils.toString(output);
            } catch (IOException e) {
                LOG.error(e);
            } finally {
                IOUtils.closeQuietly(output);
            }
        }
        return "n/a";
    }
}
