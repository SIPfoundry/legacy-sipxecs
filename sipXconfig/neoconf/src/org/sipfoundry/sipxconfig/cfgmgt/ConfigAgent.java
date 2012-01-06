/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.cfgmgt;

import static java.lang.String.format;

import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStream;
import java.io.Writer;
import java.util.Collection;

import org.apache.commons.io.IOUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.commserver.LocationsManager;

public class ConfigAgent {
    private static final Log LOG = LogFactory.getLog(ConfigManagerImpl.class);
    private String m_command;
    private String m_logDir;
    private LocationsManager m_locationsManager;

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

    void run(String command) {
        Writer err = null;
        Writer out = null;
        try {
            LOG.info("Stating agent run " + command);
            Process exec = Runtime.getRuntime().exec(command);
            err = new FileWriter(m_logDir + "/sipxagent.err.log");
            out = new FileWriter(m_logDir + "/sipxagent.out.log");
            StreamGobbler errGobbler = new StreamGobbler(exec.getErrorStream(), err);
            StreamGobbler outGobbler = new StreamGobbler(exec.getInputStream(), out);
            new Thread(errGobbler).start();
            new Thread(outGobbler).start();
            int code = exec.waitFor();
            if (code == 0) {
                if (errGobbler.m_error != null) {
                    LOG.error("Error logging error stream from agent run", errGobbler.m_error);
                }
                if (outGobbler.m_error != null) {
                    LOG.error("Error logging output stream from agent run", outGobbler.m_error);
                }
                LOG.info("Finished agent run successfully");
            } else {
                LOG.info("Agent run finshed but returned error code " + code);
            }
        } catch (InterruptedException e) {
            LOG.error("Could not complete agent command", e);
        } catch (IOException e) {
            LOG.error("Could not execute agent", e);
        } finally {
            IOUtils.closeQuietly(err);
            IOUtils.closeQuietly(out);
        }
    }

    // cfagent script will block unless streams are read
    // http://www.javaworld.com/javaworld/jw-12-2000/jw-1229-traps.html
    class StreamGobbler implements Runnable {
        private InputStream m_in;
        private Writer m_out;
        private IOException m_error;
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
}
