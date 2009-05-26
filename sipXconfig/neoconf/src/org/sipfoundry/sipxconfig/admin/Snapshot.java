/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxconfig.admin;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.Serializable;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.Collections;
import java.util.Date;
import java.util.List;
import java.util.TimeZone;
import java.util.concurrent.Callable;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;

import org.apache.commons.httpclient.HttpClient;
import org.apache.commons.httpclient.HttpException;
import org.apache.commons.httpclient.methods.GetMethod;
import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.admin.commserver.SoftwareAdminApi;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.xmlrpc.ApiProvider;
import org.sipfoundry.sipxconfig.xmlrpc.XmlRpcRemoteException;
import org.springframework.beans.factory.annotation.Required;

/**
 * Interface to command line sipx-snapshot utility
 */
public class Snapshot {
    public static final String EXECUTION_HAS_NOT_COMPLETED = "RUNNING";

    private boolean m_logs = true;

    private boolean m_credentials;

    private boolean m_cdr;

    private boolean m_profiles;

    private boolean m_www = true;

    private boolean m_filterTime = true;

    private LocationsManager m_locationsManager;

    private ApiProvider<SoftwareAdminApi> m_softwareAdminApiProvider;

    private String m_destDirectory;

    private volatile Date m_generatedDate;

    private ExecutorService m_executorService;

    private List<Future<SnapshotResult>> m_futures;

    @Required
    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }

    private String getHost() {
        return m_locationsManager.getPrimaryLocation().getFqdn();
    }

    @Required
    public void setSoftwareAdminApiProvider(ApiProvider softwareAdminApiProvider) {
        m_softwareAdminApiProvider = softwareAdminApiProvider;
    }

    public String getDestinationDirectory() {
        return m_destDirectory;
    }

    class GetSnapshot implements Callable<SnapshotResult> {
        private final Location m_location;
        private final Date m_startDate;
        private final Date m_endDate;

        public GetSnapshot(Location location, Date startDate, Date endDate) {
            super();
            m_location = location;
            m_startDate = startDate;
            m_endDate = endDate;
        }

        public SnapshotResult call() throws InterruptedException {
            HttpClient client = new HttpClient();
            String fqdn = m_location.getFqdn();
            try {
                SoftwareAdminApi api = m_softwareAdminApiProvider.getApi(m_location.getSoftwareAdminUrl());
                List<String> remoteLogFilename = api.snapshot(getHost(), getCmdLine(m_startDate, m_endDate));
                if (remoteLogFilename.isEmpty() || remoteLogFilename.get(0).length() == 0) {
                    return new SnapshotResult(new UserException("&error.xml.rpc.null.snapshot", fqdn));
                }

                do {
                    Thread.sleep(1000);
                } while (EXECUTION_HAS_NOT_COMPLETED.equals(api.execStatus(getHost(), "snapshot")));

                GetMethod httpget = new GetMethod(m_location.getHttpsServerUrl() + remoteLogFilename.get(0));

                int statusCode = client.executeMethod(httpget);
                if (statusCode != 200) {
                    return new SnapshotResult(new UserException("&error.https.server.status.code", fqdn, String
                            .valueOf(statusCode)));
                }

                File localOutputFile = new File(getDestinationDirectory(), "sipx-snapshot-" + fqdn + ".tar.gz");
                FileOutputStream fos = new FileOutputStream(localOutputFile);
                IOUtils.copy(httpget.getResponseBodyAsStream(), fos);
                fos.close();

                setGeneratedDate(new Date());

                httpget.releaseConnection();
                return new SnapshotResult(m_location, localOutputFile);
            } catch (HttpException e) {
                return new SnapshotResult(new UserException("&error.https.server", fqdn, e.getMessage()));
            } catch (IOException e) {
                return new SnapshotResult(new UserException("&error.io.exception", e.getMessage()));
            } catch (XmlRpcRemoteException e) {
                return new SnapshotResult(new UserException("&error.xml.rpc", e.getMessage(), fqdn));
            }
        }
    }

    public synchronized void perform(Date startDate, Date endDate, Location[] locations) {
        m_executorService = Executors.newFixedThreadPool(locations.length);

        m_futures = new ArrayList<Future<SnapshotResult>>(locations.length);
        for (Location location : locations) {
            if (location.isRegistered()) {
                GetSnapshot task = new GetSnapshot(location, startDate, endDate);
                m_futures.add(m_executorService.submit(task));
            }
        }
        m_executorService.shutdown();
    }

    String[] getCmdLine(Date startDate, Date endDate) {
        List<String> cmds = new ArrayList<String>();
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

        if (m_profiles) {
            cmds.add("--profiles");
        }

        if (!m_www) {
            cmds.add("--no-www");
        }

        return cmds.toArray(new String[cmds.size()]);
    }

    private String formatDate(Date date) {
        Calendar calendar = Calendar.getInstance(TimeZone.getTimeZone("UTC"));
        calendar.setTime(date);
        return (String.format("%1$tF %1$tT", calendar));
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

    public boolean isFilterTime() {
        return m_filterTime;
    }

    public void setFilterTime(boolean filterTime) {
        m_filterTime = filterTime;
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
