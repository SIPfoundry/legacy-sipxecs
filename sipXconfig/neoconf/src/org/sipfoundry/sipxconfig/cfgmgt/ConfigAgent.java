/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.cfgmgt;


import static java.lang.String.format;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStream;
import java.io.Writer;
import java.util.Collection;
import java.util.Collections;
import java.util.List;

import org.apache.commons.io.FileUtils;
import org.apache.commons.io.IOUtils;
import org.apache.commons.io.output.NullWriter;
import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.commserver.LocationsManager;

public class ConfigAgent {
    private static final Log LOG = LogFactory.getLog(ConfigManagerImpl.class);
    private String m_command;
    private String m_logDir;
    private String m_logFile = "sipxagent.log";
    private volatile boolean m_inProgress;
    private LocationsManager m_locationsManager;
    private int m_timeout = 60000;

    /**
     * synchronized to ensure cf-agent is run before last one finished, but did not
     * verify this is a strict requirement --Douglas
     */
    public synchronized void run() {
        String hosts = getHostsParams(m_locationsManager.getLocationsList());
        run(format(m_command, hosts));
    }

    String getHostsParams(Collection<Location> locations) {
        StringBuilder s = new StringBuilder();
        for (Location location : locations) {
            if (s.length() > 0) {
                s.append(' ');
            }
            s.append("-H ");
            if (location.isPrimary()) {
                // supervisord just needs to be opened to allow from this IP.
                s.append("127.0.0.1");
            } else {
                s.append(location.getAddress());
            }
        }

        return s.toString();
    }

    public List<String> getFailedItems() {
        return getLines("sipxagent-failed.log");
    }

    public List<String> getRepairedItems() {
        return getLines("sipxagent-repaired.log");
    }

    public String getLog() {
        File f = new File(m_logDir + '/' + m_logFile);
        if (f.exists()) {
            try {
                return FileUtils.readFileToString(f);
            } catch (IOException e) {
                LOG.error("cannot read log " + f.getPath(), e);
            }
        }
        return null;
    }

    @SuppressWarnings("unchecked")
    List<String> getLines(String file) {
        File f = new File(m_logDir + "/" + file);
        if (f.exists()) {
            try {
                return (List<String>) FileUtils.readLines(f);
            } catch (IOException e) {
                LOG.error("cannot read tracking log " + file, e);
            }
        }
        return Collections.emptyList();
    }

    void run(String command) {
        Writer out = null;
        Process exec = null;
        try {
            m_inProgress = true;
            LOG.info("Stating agent run " + command);
            exec = Runtime.getRuntime().exec(command);
            out = new FileWriter(m_logDir + '/' + m_logFile);
            // nothing goes to stderr, so just eat it
            StreamGobbler errGobbler = new StreamGobbler(exec.getErrorStream());
            StreamGobbler outGobbler = new StreamGobbler(exec.getInputStream(), out);
            Worker worker = new Worker(exec);
            new Thread(errGobbler).start();
            new Thread(outGobbler).start();
            Thread work = new Thread(worker);
            work.start();
            work.join(m_timeout);
            int code = worker.getExitCode();
            processResults();
            if (outGobbler.m_error != null) {
                LOG.error("Error logging output stream from agent run", outGobbler.m_error);
            }
            if (code == 0) {
                LOG.info("Finished agent run successfully");
            } else {
                throw new ConfigException("Agent run finshed but returned error code " + code);
            }
        } catch (InterruptedException e) {
            throw new ConfigException("Interrupted error. Could not complete agent command");
        } catch (IOException e) {
            throw new ConfigException("IO error. Could not complete agent command");
        } finally {
            m_inProgress = false;
            if (exec != null) {
                exec.destroy();
            }
            IOUtils.closeQuietly(out);
        }
    }

    class Worker implements Runnable {
        private Process m_process;
        private Integer m_exitCode;
        private InterruptedException m_error;
        Worker(Process process) {
            m_process = process;
        }
        public void run() {
            try {
                m_exitCode = m_process.waitFor();
            } catch (InterruptedException e) {
                m_error = e;
            }
        }

        int getExitCode() throws InterruptedException {
            if (m_error != null) {
                throw m_error;
            }
            if (m_exitCode == null) {
                throw new InterruptedException("Proccess still running");
            }
            return m_exitCode;
        }
    }

    @SuppressWarnings("serial")
    static class ConfigurationError extends Exception {
        ConfigurationError(String message) {
            super(message);
        }
    }

    private void processResults() {
        List<String> failedItems = getFailedItems();
        if (failedItems == null || failedItems.size() == 0) {
            return;
        }

        String msg = StringUtils.join(failedItems.toArray(), ",");
        throw new ConfigException("Failure in these area(s) : " + msg);
    }

    // cfagent script will block unless streams are read
    // http://www.javaworld.com/javaworld/jw-12-2000/jw-1229-traps.html
    class StreamGobbler implements Runnable {
        private InputStream m_in;
        private Writer m_out;
        private IOException m_error;
        StreamGobbler(InputStream in) {
            m_in = in;
            m_out = new NullWriter();
        }

        StreamGobbler(InputStream in, Writer out) {
            m_in = in;
            m_out = out;
        }

        @Override
        public void run() {
            try {
                IOUtils.copy(m_in, m_out);
            } catch (IOException e) {
                m_error = e;
            }
        }
    }

    public void setCommand(String command) {
        m_command = command;
    }

    public void setLogDir(String logDir) {
        m_logDir = logDir;
    }

    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }

    public boolean isInProgress() {
        return m_inProgress;
    }

    public int getTimeout() {
        return m_timeout;
    }

    public void setTimeout(int timeout) {
        m_timeout = timeout;
    }
}
