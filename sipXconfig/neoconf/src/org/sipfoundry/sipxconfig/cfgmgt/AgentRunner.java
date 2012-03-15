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
package org.sipfoundry.sipxconfig.cfgmgt;

import static java.lang.String.format;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.PipedInputStream;
import java.io.PipedOutputStream;
import java.io.Serializable;
import java.util.ArrayList;
import java.util.Collection;
import java.util.List;
import java.util.Stack;

import org.apache.commons.io.IOUtils;
import org.apache.commons.io.output.NullOutputStream;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.job.JobContext;

public class AgentRunner {
    private static final Log LOG = LogFactory.getLog(AgentRunner.class);
    private String m_command;
    private volatile boolean m_inProgress;
    private LocationsManager m_locationsManager;
    private int m_timeout = 300000;
    private JobContext m_jobContext;

    /**
     * Entry point for subclasses to all sipxagent command with a generic CFEngine task
     *
     * @param locations all the locations to run this command
     * @param label what should show in job table
     * @param subCommand does not include path to sipxagent command or host parameter
     */
    protected synchronized void run(Collection<Location> locations, String label, String subCommand) {
        try {
            m_inProgress = true;
            List<Location> ok = new ArrayList<Location>(locations.size());
            for (Location l : locations) {
                String address = l.isPrimary() ? "127.0.0.1" : l.getAddress();
                String command = format("%s --host %s %s", getCommand(), address, subCommand);
                runJob(l, label, command);
            }
        } finally {
            m_inProgress = false;
        }
    }

    /**
     * Run a full job at a location. Update job table for any failures in either running the
     * command or errors in stderr
     */
    void runJob(Location location, String label, String command) {
        PipedOutputStream log = null;
        Serializable job = m_jobContext.schedule(label, location);
        AgentResults results = new AgentResults();
        Stack<String> errs;
        try {
            m_jobContext.start(job);
            log = new PipedOutputStream();
            PipedInputStream in = new PipedInputStream(log);
            results.parse(in);
            int status = runCommand(command, log);
            errs = results.getResults(1000);
            if (errs.size() > 0) {
                String err = location.getFqdn() + ':' + errs.pop();
                ConfigManagerImpl.fail(m_jobContext, label, job, new ConfigException(err));
                while (!errs.empty()) {
                    // Tricky alert - show additional errors as new jobs
                    Serializable jobErr = m_jobContext.schedule(label, location);
                    m_jobContext.start(jobErr);
                    err = location.getFqdn() + ':' + errs.pop();
                    ConfigManagerImpl.fail(m_jobContext, label, jobErr, new ConfigException(err));
                }
            } else if (status != 0 && errs.size() == 0) {
                String msg = format("Agent run on %s finshed but returned error code %d", location.getFqdn(), status);
                ConfigManagerImpl.fail(m_jobContext, label, job, new ConfigException(msg));
            } else {
                m_jobContext.success(job);
            }
        } catch (Exception e) {
            ConfigManagerImpl.fail(m_jobContext, label, job, e);
        } finally {
            IOUtils.closeQuietly(log);
        }
    }

    /**
     * Run a command and pipe io streams accordingly
     */
    int runCommand(String command, OutputStream log) {
        Process exec = null;
        try {
            LOG.info("Starting agent run " + command);
            exec = Runtime.getRuntime().exec(command);
            StreamGobbler errGobbler = new StreamGobbler(exec.getErrorStream(), log);
            // nothing goes to stdout, so just eat it
            StreamGobbler outGobbler = new StreamGobbler(exec.getInputStream());
            Worker worker = new Worker(exec);
            Thread err = new Thread(errGobbler);
            err.start();
            new Thread(outGobbler).start();
            Thread work = new Thread(worker);
            work.start();
            work.join(m_timeout);
            err.join(1000);
            if (errGobbler.m_error != null) {
                LOG.error("Error logging output stream from agent run", outGobbler.m_error);
            }
            return worker.getExitCode();
        } catch (InterruptedException e) {
            throw new ConfigException(format("Interrupted error. Could not complete agent command in %d ms.",
                    m_timeout));
        } catch (IOException e) {
            throw new ConfigException("IO error. Could not complete agent command " + e.getMessage());
        } finally {
            if (exec != null) {
                exec.destroy();
            }
        }
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

    // cfagent script will block unless streams are read
    // http://www.javaworld.com/javaworld/jw-12-2000/jw-1229-traps.html
    class StreamGobbler implements Runnable {
        private InputStream m_in;
        private OutputStream m_out;
        private IOException m_error;

        StreamGobbler(InputStream in) {
            m_in = in;
            m_out = new NullOutputStream();
        }

        StreamGobbler(InputStream in, OutputStream out) {
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

    public LocationsManager getLocationsManager() {
        return m_locationsManager;
    }

    public void setCommand(String command) {
        m_command = command;
    }

    public String getCommand() {
        return m_command;
    }

    public void setJobContext(JobContext jobContext) {
        m_jobContext = jobContext;
    }
}
