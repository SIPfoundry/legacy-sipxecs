/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.bulk.ldap;

import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.ScheduledFuture;
import java.util.concurrent.TimeUnit;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.common.ApplicationInitializedEvent;
import org.sipfoundry.sipxconfig.common.CronSchedule;
import org.sipfoundry.sipxconfig.common.DSTChangeEvent;
import org.springframework.context.ApplicationEvent;
import org.springframework.context.ApplicationListener;


public class LdapImportTrigger implements ApplicationListener {
    private static final Log LOG = LogFactory.getLog("ldap");

    private LdapManager m_ldapManager;

    private LdapImportManager m_ldapImportManager;

    private Map<Integer, ScheduledFuture<?>> m_taskHash = new HashMap<Integer, ScheduledFuture<?>>();

    private ScheduledExecutorService m_executor = Executors.newSingleThreadScheduledExecutor();

    public void setLdapManager(LdapManager ldapManager) {
        m_ldapManager = ldapManager;
    }

    public void setLdapImportManager(LdapImportManager ldapImportManager) {
        m_ldapImportManager = ldapImportManager;
    }

    /**
     * start timers after app is initialized
     */
    public void onApplicationEvent(ApplicationEvent event) {
        if (event instanceof ApplicationInitializedEvent || event instanceof DSTChangeEvent) {
            List<LdapConnectionParams> params = m_ldapManager.getAllConnectionParams();
            for (LdapConnectionParams conParams : params) {
                CronSchedule schedule = conParams.getSchedule();
                onScheduleChanged(schedule, conParams.getId());
            }
        } else if (event instanceof ScheduleChangedEvent) {
            ScheduleChangedEvent sce = (ScheduleChangedEvent) event;
            onScheduleChanged(sce.getSchedule(), sce.getConnectionId());
        } else if (event instanceof ScheduleDeletedEvent) {
            ScheduleDeletedEvent sde = (ScheduleDeletedEvent) event;
            onScheduleDeleted(sde.getConnectionId());
        }
    }

    private synchronized void onScheduleChanged(CronSchedule schedule, int connectionId) {
        ScheduledFuture<?> currentFuture = m_taskHash.get(connectionId);
        if (currentFuture != null) {
            currentFuture.cancel(false);
        }
        Runnable ldapImportTask = new LdapImportTask(m_ldapImportManager, connectionId);
        ScheduledFuture<?> newFuture = schedule.schedule(m_executor, ldapImportTask);
        m_taskHash.put(connectionId, newFuture);
    }

    private synchronized void onScheduleDeleted(int connectionId) {
        ScheduledFuture<?> timerTask = m_taskHash.get(connectionId);
        if (timerTask != null) {
            timerTask.cancel(false);
        }
        m_taskHash.remove(connectionId);
    }

    public boolean isScheduledImportRunning() {
        for (ScheduledFuture<?> future : m_taskHash.values()) {
            if (future != null && future.getDelay(TimeUnit.MILLISECONDS) <= 0) {
                return true;
            }
        }
        return false;
    }

    public static final class ScheduleChangedEvent extends ApplicationEvent {
        private CronSchedule m_schedule;
        private int m_connectionId;

        public ScheduleChangedEvent(CronSchedule schedule, Object eventSource, int connectionId) {
            super(eventSource);
            m_schedule = schedule;
            m_connectionId = connectionId;
        }

        public CronSchedule getSchedule() {
            return m_schedule;
        }

        public int getConnectionId() {
            return m_connectionId;
        }
    }

    public static final class ScheduleDeletedEvent extends ApplicationEvent {
        private int m_connectionId;

        public ScheduleDeletedEvent(Object eventSource, int connectionId) {
            super(eventSource);
            m_connectionId = connectionId;
        }

        public int getConnectionId() {
            return m_connectionId;
        }
    }

    private static final class LdapImportTask implements Runnable {
        private LdapImportManager m_ldapImportManager;
        private int m_connectionId;

        public LdapImportTask(LdapImportManager ldapImportManager, int connectionId) {
            m_ldapImportManager = ldapImportManager;
            m_connectionId = connectionId;
        }

        public void run() {
            LOG.info("Ready to start scheduled import");
            m_ldapImportManager.insert(m_connectionId);
        }
    }
}
