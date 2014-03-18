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

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.Serializable;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import org.apache.commons.io.IOUtils;
import org.apache.commons.io.output.NullOutputStream;
import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.job.JobContext;

/**
 * Run commands and allow caller to block for a specific time and unblock for
 * a specific remaining time before killing process.
 */
public class SimpleCommandRunner implements CommandRunner {
    private static final Log LOG = LogFactory.getLog(SimpleCommandRunner.class);
    private ByteArrayOutputStream m_stderr;
    private ByteArrayOutputStream m_stdout;
    private String m_stdin;
    private Thread m_inThread;
    private Thread m_outThread;
    private Thread m_errThread;
    private Thread m_procThread;
    private volatile Integer m_exitCode;
    private String[] m_command;
    private int m_foregroundTimeout;
    private int m_backgroundTimeout;
    private JobContext m_jobContext;
    private String m_jobName;

    public SimpleCommandRunner(JobContext jobContext) {
        m_jobContext = jobContext;
    }

    public SimpleCommandRunner() {
        m_jobContext = null;
        m_jobName = null;
    }

    public String getStderr() {
        return m_stderr.toString();
    }

    public String getStdout() {
        return m_stdout.toString();
    }

    public void setStdin(String stdin) {
        m_stdin = stdin;
    }

    public String getStdin() {
        return m_stdin;
    }

    public Integer getExitCode() {
        return m_exitCode;
    }

    public boolean isInProgress() {
        return m_procThread != null && m_procThread.isAlive();
    }

    /**
     * Command will not go into background
     */
    public void setRunParameters(String command, int foregroundTimeout) {
        setRunParameters(command, foregroundTimeout, 0);
    }

    /**
     * If command takes longer then foregroundTimeout, call will return and command
     * will run in background until  backgroundTimeout at which time it will abort mission
     */
    public void setRunParameters(String command, int foregroundTimeout, int backgroundTimeout) {
        m_command = split(command);
        m_foregroundTimeout = foregroundTimeout;
        m_backgroundTimeout = backgroundTimeout;
    }

    public boolean run(String[] command, int foregroundTimeout) {
        return run(command, foregroundTimeout, 0);
    }

    public boolean run(String[] command, int foregroundTimeout, int backgroundTimeout) {
        m_command = command;
        m_foregroundTimeout = foregroundTimeout;
        m_backgroundTimeout = backgroundTimeout;
        return run();
    }

    public boolean run() {
        // clean-up in case this is reused
        kill();
        m_stderr = new ByteArrayOutputStream();
        m_stdout = new ByteArrayOutputStream();
        m_exitCode = null;
        m_inThread = null;

        final String fullCommand = StringUtils.join(m_command, ' ');
        LOG.info(fullCommand);
        ProcessBuilder pb = new ProcessBuilder(Arrays.asList(m_command));
        final Serializable job;
        try {
            if (m_jobContext != null && m_jobName != null) {
                job = m_jobContext.schedule(m_jobName);
                m_jobContext.start(job);
            } else {
                job = null;
            }
            final Process p = pb.start();
            if (m_stdin != null) {
                Runnable inPutter = new Runnable() {
                    @Override
                    public void run() {
                        InputStream in = new ByteArrayInputStream(m_stdin.getBytes());
                        try {
                            IOUtils.copy(in, p.getOutputStream());
                            in.close();
                            // need to close otherwise process hangs.
                            p.getOutputStream().close();
                        } catch (IOException e) {
                            LOG.error("Closing input stream of command runner. " + fullCommand, e);
                        }
                    }
                };
                m_inThread = new Thread(null, inPutter, "CommandRunner-stdin");
                m_inThread.start();
            }
            StreamGobbler outGobbler = new StreamGobbler(p.getInputStream(), m_stdout);
            m_outThread = new Thread(null, outGobbler, "CommandRunner-stdout");
            m_outThread.start();
            StreamGobbler errGobbler = new StreamGobbler(p.getErrorStream(), m_stderr);
            m_errThread = new Thread(null, errGobbler, "CommandRunner-stderr");
            m_errThread.start();
            Runnable procRunner = new Runnable() {
                @Override
                public void run() {
                    try {
                        m_exitCode = p.waitFor();
                        if (job != null) {
                            if (m_exitCode == 0) {
                                m_jobContext.success(job);
                            } else {
                                m_jobContext.failure(job, m_jobName + " failed", null);
                            }
                        }
                    } catch (InterruptedException willBeHandledByReaper) {
                        LOG.error("Interrupted running " + fullCommand);
                    } finally {
                        synchronized (SimpleCommandRunner.this) {
                            SimpleCommandRunner.this.notifyAll();
                        }
                    }
                }
            };
            m_procThread = new Thread(null, procRunner, "CommandRunner-process");
            m_procThread.start();

            m_procThread.join(m_foregroundTimeout);

            // all ok
            if (m_exitCode != null) {
                m_outThread.join();
                m_errThread.join();
                return true;
            }

            // not requested to go into background
            if (m_backgroundTimeout <= m_foregroundTimeout) {
                LOG.info("background timer not specified or valid, killing process " + fullCommand);
                if (job != null) {
                    m_jobContext.failure(job, "Interrupted(foreground) " + m_jobName, null);
                }
                kill();
                return false;
            }

            final int remainingTime = m_backgroundTimeout - m_foregroundTimeout;
            LOG.info("putting process in background for " + remainingTime + " ms " + fullCommand);
            // schedule process to be killed after background timeout
            Runnable reaper = new Runnable() {
                @Override
                public void run() {
                    try {
                        // already waited foreground, so subtract that off
                        m_procThread.join(remainingTime);
                        if (m_exitCode == null) {
                            LOG.info("Reaping background process, did not complete in time. " + fullCommand);
                            if (job != null) {
                                m_jobContext.failure(job, "Interrupted(background) " + m_jobName, null);
                            }
                            kill();
                        }
                    } catch (InterruptedException e) {
                        throw new UserException(e);
                    }
                }
            };
            Thread reaperThread = new Thread(null, reaper, "CommandRunner-reaper");
            reaperThread.start();

            // just in case notify from finishing process didn't get called
            synchronized (this) {
                notifyAll();
            }

            return false;
        } catch (IOException e) {
            throw new UserException(e);
        } catch (InterruptedException e1) {
            throw new UserException(e1);
        }
    }

    void kill() {
        for (Thread t : new Thread[] {
            m_inThread, m_procThread, m_errThread, m_outThread
        }) {
            if (t != null && t.isAlive()) {
                t.interrupt();
            }
        }
        m_inThread = null;
        m_procThread = null;
        m_errThread = null;
        m_outThread = null;
    }

    public void setJobName(String jobName) {
        m_jobName = jobName;
    }

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

    public static String[] split(String command) {
        StringBuilder param = new StringBuilder();
        List<String> cmd = new ArrayList<String>();
        boolean quoted = false;
        for (int i = 0; i < command.length(); i++) {
            char c = command.charAt(i);
            switch (c) {
            case ' ':
                if (quoted) {
                    param.append(c);
                } else {
                    commandSplitTakeParam(cmd, param);
                }
                break;
            case '\'':
                if (quoted) {
                    commandSplitTakeParam(cmd, param);
                }
                quoted = !quoted;
                break;
            default:
                param.append(c);
            }
            if (i == command.length() - 1) {
                commandSplitTakeParam(cmd, param);
            }
        }
        return cmd.toArray(new String[0]);
    }

    static void commandSplitTakeParam(List<String> cmd, StringBuilder param) {
        if (param.length() > 0) {
            cmd.add(param.toString());
            param.setLength(0);
        }
    }
}
