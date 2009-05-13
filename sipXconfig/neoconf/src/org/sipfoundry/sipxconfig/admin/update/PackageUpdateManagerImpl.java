/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */

package org.sipfoundry.sipxconfig.admin.update;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;
import java.util.concurrent.Callable;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;

import org.apache.commons.collections.CollectionUtils;
import org.sipfoundry.sipxconfig.common.AlarmContext;
import org.sipfoundry.sipxconfig.common.UserException;
import org.springframework.beans.factory.annotation.Required;

public class PackageUpdateManagerImpl implements Serializable, PackageUpdateManager {
    private UpdateApi m_updateApi;

    /** Executor that runs the package update tasks in the background. */
    private final ExecutorService m_updateExecutor = Executors.newSingleThreadExecutor();

    /** The current state of the updater. */
    private UpdaterState m_state = UpdaterState.NO_UPDATES_AVAILABLE;

    /** The current list of available sipXecs package updates. */
    private List<PackageUpdate> m_availablePackages = new ArrayList<PackageUpdate>();

    /** The last time we checked for updates. */
    private Date m_lastCheckedDate;

    /** The currently installed sipx version. */
    private String m_currentVersion;

    /** The available updated sipx version. */
    private String m_updatedVersion;

    private AlarmContext m_alarmContext;

    private boolean m_yumCapable;

    public List<PackageUpdate> getAvailablePackages() {
        return m_availablePackages;
    }

    public Date getLastCheckedDate() {
        return m_lastCheckedDate;
    }

    public UpdaterState getState() {
        return m_state;
    }

    public String getUpdatedVersion() {
        return m_updatedVersion;
    }

    public Future<Boolean> checkForUpdates() {
        m_state = UpdaterState.CHECKING_FOR_UPDATES;
        return m_updateExecutor.submit(new CheckForUpdatesTask());
    }

    public Future< ? > installUpdates() {
        m_state = UpdaterState.INSTALLING;
        Runnable task = new Runnable() {
            public void run() {
                m_updateApi.installUpdates();
                m_state = UpdaterState.UPDATE_COMPLETED;
            }
        };
        return m_updateExecutor.submit(task);
    }

    public String getCurrentVersion() {
        if (m_currentVersion != null) {
            return m_currentVersion;
        }
        Callable<String> task = new Callable<String>() {
            public String call() throws Exception {
                return m_updateApi.getCurrentVersion();
            }
        };
        Future<String> future = m_updateExecutor.submit(task);
        try {
            m_currentVersion = future.get();
        } catch (InterruptedException e) {
            throw new RuntimeException(e);
        } catch (ExecutionException e) {
            throw new UserException(e);
        }
        return m_currentVersion;
    }

    private String buildAvailablePackagesString() {
        StringBuilder availablePackages = new StringBuilder();
        availablePackages.append("# Package Name|Installed Version|Updated Version\n");
        for (PackageUpdate packageUpdate : m_availablePackages) {
            availablePackages.append(packageUpdate).append('\n');
        }
        return availablePackages.toString();
    }

    /**
     * This task checks for updated sipxecs packages and updates the UI accordingly.
     */
    class CheckForUpdatesTask implements Callable<Boolean> {
        public Boolean call() throws Exception {
            m_lastCheckedDate = new Date();
            m_availablePackages = m_updateApi.getAvailableUpdates();
            if (CollectionUtils.isEmpty(m_availablePackages)) {
                m_state = UpdaterState.NO_UPDATES_AVAILABLE;
            } else {
                m_state = UpdaterState.UPDATES_AVAILABLE;
                String alarmData = buildAvailablePackagesString();
                m_alarmContext.raiseAlarm("SOFTWARE_UPDATE_AVAILABLE", alarmData);
            }

            for (PackageUpdate update : m_availablePackages) {
                if (update.getPackageName().equals("sipxecs")) {
                    m_updatedVersion = String.format("sipxecs %s", update.getUpdatedVersion());
                }
            }
            return !m_availablePackages.isEmpty();
        }
    }

    @Required
    public void setUpdateApi(UpdateApi updateApi) {
        m_updateApi = updateApi;
    }

    @Required
    public void setAlarmContext(AlarmContext alarmContext) {
        m_alarmContext = alarmContext;
    }

    public void setYumCapable(boolean yumCapable) {
        m_yumCapable = yumCapable;
    }

    public boolean isYumCapable() {
        return m_yumCapable;
    }
}
