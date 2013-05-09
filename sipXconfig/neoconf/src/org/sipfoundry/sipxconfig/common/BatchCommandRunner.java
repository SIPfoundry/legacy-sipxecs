/**
 * Copyright (c) 2013 eZuce, Inc. All rights reserved.
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

import java.util.ArrayList;
import java.util.List;

import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

/**
 * Run a list of other command runners as if it's one command
 */
public class BatchCommandRunner implements CommandRunner {
    private static final Log LOG = LogFactory.getLog(BatchCommandRunner.class);
    private List<CommandRunner> m_runners = new ArrayList<CommandRunner>();
    private StringBuilder m_out = new StringBuilder();
    private StringBuilder m_err = new StringBuilder();
    private int m_foregroundTimeout;
    private int m_backgroundTimeout;
    private Integer m_exitCode;
    private Thread m_runner;
    private String m_label;

    /**
     * @param label - Used for logging purposes
     */
    public BatchCommandRunner(String label) {
        m_label = label;
    }

    public void add(CommandRunner runner) {
        m_runners.add(runner);
    }

    public boolean isInProgress() {
        return getActiveJob() != null;
    }

    public CommandRunner getActiveJob() {
        if (!m_runners.isEmpty()) {
            for (CommandRunner runner : m_runners) {
                if (runner.isInProgress()) {
                    return runner;
                }
            }
        }
        return null;
    }

    public boolean run() {
        m_runner = new Thread() {
            @Override
            public void run() {
                Integer exitCode = null;
                for (CommandRunner job : m_runners) {
                    if (!job.run()) {
                        try {
                            synchronized (job) {
                                job.wait();
                            }
                        } catch (InterruptedException e) {
                            LOG.error(e);
                        }
                    }
                    String jobOut = job.getStdout();
                    if (StringUtils.isNotBlank(jobOut)) {
                        m_out.append(jobOut);
                    }

                    exitCode = job.getExitCode();
                    if (exitCode != 0) {
                        String jobErr = job.getStderr();
                        if (StringUtils.isNotBlank(jobErr)) {
                            m_err.append(jobErr);
                        }
                        break;
                    }
                }
                m_exitCode = exitCode;
                synchronized (BatchCommandRunner.this) {
                    BatchCommandRunner.this.notifyAll();
                }
            };
        };
        m_runner.start();
        try {
            synchronized (m_runner) {
                m_runner.join(m_foregroundTimeout);
                if (m_exitCode != null) {
                    return true;
                }
            }
        } catch (InterruptedException e) {
            LOG.error(e);
        }

        final int remainingTime = m_backgroundTimeout - m_foregroundTimeout;
        if (remainingTime > 0) {
            LOG.info("putting " + m_label + " batch process in background for " + remainingTime + " ms");
            // schedule process to be killed after background timeout
            Runnable reaper = new Runnable() {
                @Override
                public void run() {
                    try {
                        // already waited foreground, so subtract that off
                        m_runner.join(remainingTime);
                        if (m_exitCode == null) {
                            LOG.info("Reaping " + m_label + " background process, did not complete in time.");
                            kill();
                        }
                    } catch (InterruptedException e) {
                        throw new UserException(e);
                    }
                }
            };
            Thread reaperThread = new Thread(null, reaper, "BatchCommandRunner-reaper");
            reaperThread.start();
        } else {
            kill();
        }

        // just in case notify from finishing process didn't get called
        synchronized (this) {
            notifyAll();
        }

        return false;
    }

    void kill() {
        if (m_runner != null && m_runner.isAlive()) {
            m_runner.interrupt();
        }
        m_runner = null;
    }

    public void setForegroundTimeout(int foregroundTimeout) {
        m_foregroundTimeout = foregroundTimeout;
    }

    public void setBackgroundTimeout(int backgroundTimeout) {
        m_backgroundTimeout = backgroundTimeout;
    }

    @Override
    public String getStdout() {
        return m_out.toString();
    }

    @Override
    public Integer getExitCode() {
        return m_exitCode;
    }

    @Override
    public String getStderr() {
        return m_err.toString();
    }
}
