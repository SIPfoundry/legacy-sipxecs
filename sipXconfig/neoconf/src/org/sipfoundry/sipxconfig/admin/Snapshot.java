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
import java.util.Calendar;
import java.util.Date;
import java.util.List;
import java.util.TimeZone;

import org.sipfoundry.sipxconfig.common.UserException;

/**
 * Interface to command line sipx-snapshot utility
 */
public class Snapshot {
    public static final String RESULT_FILE_NAME = "sipx-configuration.tar.gz";

    private boolean m_logs = true;

    private boolean m_credentials;

    private boolean m_cdr;

    private boolean m_www = true;

    private boolean m_filterTime;

    private String m_binDirectory;

    private String m_destDirectory;

    public File perform(Date startDate, Date endDate) {
        try {
            File destDir = new File(m_destDirectory);
            Runtime runtime = Runtime.getRuntime();
            Process process = runtime.exec(getCmdLine(m_binDirectory, startDate, endDate), null, destDir);
            process.waitFor();
            int exitValue = process.exitValue();
            if (exitValue != 0) {
                throw new UserException("Errors when executing snapshot script: {0}", String.valueOf(exitValue));
            }
            return new File(destDir, RESULT_FILE_NAME);
        } catch (IOException e) {
            throw new UserException("Cannot retrieve configuration snapshot.\n{0}", e.getLocalizedMessage());
        } catch (InterruptedException e) {
            throw new UserException("Snapshot service inexpectedly terminated.\n{0}", e.getLocalizedMessage());
        }
    }

    String[] getCmdLine(String binDirectory, Date startDate, Date endDate) {
        File executable = new File(binDirectory, "sipx-snapshot");
        List<String> cmds = new ArrayList<String>();
        cmds.add(executable.getPath());
        cmds.add("--logs");
        if (m_logs) {
            cmds.add("current");
            // Log start/stop times may only be specified with '--logs current'
            if (m_filterTime) {
                // Times must be specified in UCT
                cmds.add("--log-start");
                cmds.add(formatDate(startDate));
                cmds.add("--log-stop");
                cmds.add(formatDate(endDate));
            }
        } else {
            cmds.add("none");
        }

        if (m_credentials) {
            cmds.add("--credentials");
        }

        if (m_cdr) {
            cmds.add("--cdr");
        }

        if (!m_www) {
            cmds.add("--no-www");
        }

        cmds.add(RESULT_FILE_NAME);
        return cmds.toArray(new String[cmds.size()]);
    }

    private String formatDate(Date date) {
        Calendar calendar = Calendar.getInstance(TimeZone.getTimeZone("UTC"));
        calendar.setTime(date);
        return String.format("'%1$tF %1$tT'", calendar);
    }

    public boolean isCredentials() {
        return m_credentials;
    }

    public void setCredentials(boolean credentials) {
        m_credentials = credentials;
    }

    public boolean isCdr() {
        return m_cdr;
    }

    public void setCdr(boolean cdr) {
        m_cdr = cdr;
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

    public boolean isFilterTime() {
        return m_filterTime;
    }

    public void setFilterTime(boolean filterTime) {
        m_filterTime = filterTime;
    }

    public void setBinDirectory(String binDirectory) {
        m_binDirectory = binDirectory;
    }

    public void setDestDirectory(String destDirectory) {
        m_destDirectory = destDirectory;
    }
}
