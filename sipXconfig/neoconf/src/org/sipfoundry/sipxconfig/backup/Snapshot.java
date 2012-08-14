/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxconfig.backup;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.Serializable;
import java.io.Writer;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Date;
import java.util.List;
import java.util.concurrent.Callable;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigCommands;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.commserver.LocationsManager;
import org.springframework.beans.factory.annotation.Required;

/**
 * Interface to command line sipx-snapshot utility
 */
public class Snapshot {
    public static final String EXECUTION_HAS_NOT_COMPLETED = "RUNNING";

    private static final String SEPARATOR = " ";

    private boolean m_logs = true;

    private boolean m_credentials;

    private boolean m_cdr;

    private boolean m_profiles;

    private boolean m_www = true;

    private boolean m_logFilter = true;

    private int m_lines = 1000;

    private LocationsManager m_locationsManager;

    private String m_destDirectory;

    private volatile Date m_generatedDate;

    private ExecutorService m_executorService;

    private List<Future<SnapshotResult>> m_futures;

    private ConfigCommands m_configCommands;

    @Required
    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }

    public String getDestinationDirectory() {
        return m_destDirectory;
    }

    @Required
    public void setConfigCommands(ConfigCommands configCommands) {
        m_configCommands = configCommands;
    }

    class GetSnapshot implements Callable<SnapshotResult> {
        private final Location m_location;
        private final ConfigCommands m_configCommands;

        public GetSnapshot(Location location, ConfigCommands configCommands) {
            super();
            m_location = location;
            m_configCommands = configCommands;
        }

        public SnapshotResult call() throws InterruptedException {
            m_configCommands.collectSnapshot(m_location);
            setGeneratedDate(new Date());
            if (!m_location.isPrimary()) {
                // get snapshot from distributed
                m_configCommands.uploadSnapshot(m_locationsManager.getPrimaryLocation());
            }
            return new SnapshotResult(m_location, new File(getDestinationDirectory(),
                    getArchiveName(m_location.getFqdn())));
        }
    }

    public synchronized void perform(Location[] locations, int lines) {
        m_executorService = Executors.newFixedThreadPool(locations.length);

        m_futures = new ArrayList<Future<SnapshotResult>>(locations.length);
        for (Location location : locations) {
            Writer wtr = null;
            try {
                File f = new File(((ConfigManager) m_configCommands).getLocationDataDirectory(location),
                        "snapshot.ini");
                wtr = new FileWriter(f);
                composeCmdLine(wtr, location.getFqdn(), lines);
            } catch (IOException ex) {
                throw new UserException("&err.snapshot.failed");
            } finally {
                IOUtils.closeQuietly(wtr);
            }
            GetSnapshot task = new GetSnapshot(location, m_configCommands);
            m_futures.add(m_executorService.submit(task));
        }
        m_executorService.shutdown();
    }

    void composeCmdLine(Writer wtr, String fqdn, int lines) throws IOException {
        wtr.write("--logs");
        if (m_logs) {
            wtr.write(SEPARATOR);
            wtr.write("current");
            // 'Log filter' may only be specified with '--logs current'
            if (m_logFilter) {
                wtr.write(SEPARATOR);
                wtr.write("--lines");
                wtr.write(SEPARATOR);
                wtr.write(Integer.toString(lines));
            }
        } else {
            wtr.write(SEPARATOR);
            wtr.write("none");
        }

        if (m_credentials) {
            wtr.write(SEPARATOR);
            wtr.write("--credentials");
        }

        if (m_cdr) {
            wtr.write(SEPARATOR);
            wtr.write("--cdr");
        }

        if (m_profiles) {
            wtr.write(SEPARATOR);
            wtr.write("--profiles");
        }

        if (!m_www) {
            wtr.write(SEPARATOR);
            wtr.write("--no-www");
        }

        wtr.write(SEPARATOR);
        wtr.write(getDestinationDirectory() + "/" + getArchiveName(fqdn));
    }

    private String getArchiveName(String fqdn) {
        return String.format("sipx-snapshot-%s.tar.gz", fqdn);
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

    public boolean isProfiles() {
        return m_profiles;
    }

    public void setProfiles(boolean profiles) {
        m_profiles = profiles;
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

    public boolean isLogFilter() {
        return m_logFilter;
    }

    public void setLogFilter(boolean logFilter) {
        m_logFilter = logFilter;
    }

    public int getLines() {
        return m_lines;
    }

    public void setLines(int lines) {
        m_lines = lines;
    }

    public void setDestDirectory(String destDirectory) {
        m_destDirectory = destDirectory;
    }

    public Date getGeneratedDate() {
        return m_generatedDate;
    }

    public void setGeneratedDate(Date generatedDate) {
        m_generatedDate = generatedDate;
    }

    /**
     * Executor service has been created and is now in running (not terminated) state.
     */
    public synchronized boolean isRefreshing() {
        return isInitialized() && !m_executorService.isTerminated();
    }

    /**
     * Executor service has been created.
     */
    public synchronized boolean isInitialized() {
        return m_executorService != null;
    }

    /**
     * Returns 'true' if at least one SnapshotResult bean was successfully retrieved
     */
    public synchronized boolean isSuccess() {
        List<SnapshotResult> results = getResults();
        for (SnapshotResult result : results) {
            if (result.isSuccess()) {
                return true;
            }
        }
        return false;
    }

    public synchronized List<SnapshotResult> getResults() {
        if (isRefreshing()) {
            return Collections.emptyList();
        }
        if (m_futures == null) {
            return Collections.emptyList();
        }
        List<SnapshotResult> results = new ArrayList<SnapshotResult>(m_futures.size());
        try {
            for (Future<SnapshotResult> future : m_futures) {
                results.add(future.get());
            }
            return results;
        } catch (InterruptedException e) {
            throw new UserException(e);
        } catch (ExecutionException e) {
            // we handle exceptions in task already so this should not happen
            throw new UserException(e);
        }
    }

    public static class SnapshotResult implements Serializable {
        private final File m_snaphotFile;
        private final String m_fqdn;
        private final UserException m_userException;

        public SnapshotResult(Location location, File snapshotFile) {
            m_fqdn = location.getFqdn();
            m_snaphotFile = snapshotFile;
            m_userException = null;
        }

        public SnapshotResult(UserException userException) {
            m_fqdn = null;
            m_snaphotFile = null;
            m_userException = userException;
        }

        public UserException getUserException() {
            return m_userException;
        }

        public boolean isSuccess() {
            return m_userException == null;
        }

        public String getFqdn() {
            return m_fqdn;
        }

        public File getFile() {
            return m_snaphotFile;
        }

        public String getDir() {
            return m_snaphotFile.getParent();
        }
    }
}
