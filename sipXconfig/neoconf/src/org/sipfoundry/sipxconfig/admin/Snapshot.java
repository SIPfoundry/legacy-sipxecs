/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.admin;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

import org.sipfoundry.sipxconfig.common.UserException;

/**
 * Interface to command line sipx-snapshot utility
 */
public class Snapshot {
    public static final String RESULT_FILE_NAME = "sipx-configuration.tar.gz";

    private boolean m_logs = true;

    private boolean m_credentials;

    private boolean m_www = true;

    private String m_binDirectory;

    private String m_destDirectory;

    public File perform() {
        try {
            File destDir = new File(m_destDirectory);
            Runtime runtime = Runtime.getRuntime();
            Process process = runtime.exec(getCmdLine(m_binDirectory), null, destDir);
            process.waitFor();
            int exitValue = process.exitValue();
            if (exitValue != 0) {
                throw new UserException("Errors when executing snapshot script: {0}", String
                        .valueOf(exitValue));
            }
            return new File(destDir, RESULT_FILE_NAME);
        } catch (IOException e) {
            throw new UserException("Cannot retrieve configuration snapshot.\n{0}", e
                    .getLocalizedMessage());
        } catch (InterruptedException e) {
            throw new UserException("Snapshot service inexpectedly terminated.\n{0}", e
                    .getLocalizedMessage());
        }
    }

    String[] getCmdLine(String binDirectory) {
        File executable = new File(binDirectory, "sipx-snapshot");
        List cmds = new ArrayList();
        cmds.add(executable.getPath());
        cmds.add("--logs");
        cmds.add(m_logs ? "current" : "none");
        if (m_credentials) {
            cmds.add("--credentials");

        }
        if (!m_www) {
            cmds.add("--no-www");
        }
        cmds.add(RESULT_FILE_NAME);
        return (String[]) cmds.toArray(new String[cmds.size()]);
    }

    public boolean isCredentials() {
        return m_credentials;
    }

    public void setCredentials(boolean credentials) {
        m_credentials = credentials;
    }

    public boolean isLogs() {
        return m_logs;
    }

    public void setLogs(boolean logs) {
        m_logs = logs;
    }

    public boolean isWww() {
        return m_www;
    }

    public void setWww(boolean www) {
        m_www = www;
    }

    public void setBinDirectory(String binDirectory) {
        m_binDirectory = binDirectory;
    }

    public void setDestDirectory(String destDirectory) {
        m_destDirectory = destDirectory;
    }
}
