/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.commserver;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;

import org.apache.commons.io.FileUtils;
import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.common.UserException;
import org.springframework.beans.factory.annotation.Required;

public class InitialConfig {
    private static final String INITIAL_CONFIG = "/initial-config";
    private static final Log LOG = LogFactory.getLog(InitialConfig.class);

    private String m_tmpDirectory;
    private String m_binDirectory;

    /**
     * Set the directory where the initial-config script is located. Note that despite the name,
     * this may not acutally be the "bin" dir
     */
    @Required
    public void setBinDirectory(String binDirectory) {
        m_binDirectory = binDirectory;
    }

    @Required
    public void setTmpDirectory(String tmpDirectory) {
        m_tmpDirectory = tmpDirectory;
    }

    public String getBinDirectory() {
        return m_binDirectory;
    }

    public String getTmpDirectory() {
        return m_tmpDirectory;
    }

    public InputStream getArchiveStream(String location) throws IOException {
        // create the archive
        createArchive(location);
        // get input stream
        InputStream stream = new FileInputStream(new File(m_tmpDirectory + INITIAL_CONFIG + "/" + location
                + ".tar.gz"));
        return stream;
    }

    public void deleteInitialConfigDirectory() {
        try {
            FileUtils.deleteDirectory(new File(m_tmpDirectory + INITIAL_CONFIG));
        } catch (IOException ex) {
            LOG.error("Could not delete initial-config directory", ex);
        }
    }

    private void createArchive(String locationName) {
        try {

            String[] cmdLine = new String[] {
                m_binDirectory + INITIAL_CONFIG, locationName
            };
            ProcessBuilder pb = new ProcessBuilder(cmdLine);
            java.lang.Process proc = pb.directory(new File(m_tmpDirectory)).start();
            LOG.debug("Executing: " + StringUtils.join(cmdLine, " "));
            proc.waitFor();

            if (proc.exitValue() != 0) {
                throw new UserException("Script finished with exit code: " + proc.exitValue());
            }
        } catch (IOException e) {
            throw new UserException(e);
        } catch (InterruptedException e) {
            throw new UserException(e);
        }
    }
}
