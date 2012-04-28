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

import java.util.List;
import java.util.Timer;
import java.util.TimerTask;

import org.sipfoundry.sipxconfig.common.ApplicationInitializedEvent;
import org.sipfoundry.sipxconfig.common.CronSchedule;
import org.sipfoundry.sipxconfig.common.DSTChangeEvent;
import org.springframework.context.ApplicationEvent;
import org.springframework.context.ApplicationListener;

public class LdapImportTrigger implements ApplicationListener {
    private LdapManager m_ldapManager;

    private LdapImportManager m_ldapImportManager;

    private Timer m_timer;

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
        }
    }

    private synchronized void onScheduleChanged(CronSchedule schedule, int connectionId) {
        if (m_timer != null) {
            m_timer.cancel();
        }
        TimerTask ldapImportTask = new LdapImportTask(m_ldapImportManager, connectionId);
        m_timer = schedule.schedule(ldapImportTask);
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

    private static final class LdapImportTask extends TimerTask {
        private LdapImportManager m_ldapImportManager;
        private int m_connectionId;

        public LdapImportTask(LdapImportManager ldapImportManager, int connectionId) {
            m_ldapImportManager = ldapImportManager;
            m_connectionId = connectionId;
        }

        public void run() {
            m_ldapImportManager.insert(m_connectionId);
        }
    }

}
