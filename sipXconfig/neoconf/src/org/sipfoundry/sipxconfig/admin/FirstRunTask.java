/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.admin;

import java.util.List;

import org.apache.commons.lang.ArrayUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessContext;
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanActivatedEvent;
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanContext;
import org.sipfoundry.sipxconfig.common.AlarmContext;
import org.sipfoundry.sipxconfig.common.ApplicationInitializedEvent;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.springframework.beans.factory.annotation.Required;
import org.springframework.context.ApplicationEvent;
import org.springframework.context.ApplicationListener;

public class FirstRunTask implements ApplicationListener {
    private static final Log LOG = LogFactory.getLog(FirstRunTask.class);

    private CoreContext m_coreContext;
    private AdminContext m_adminContext;
    private DomainManager m_domainManager;
    private DialPlanContext m_dialPlanContext;
    private SipxProcessContext m_processContext;
    private String m_taskName;
    private AlarmContext m_alarmContext;

    public void runTask() {
        LOG.info("Executing first run tasks...");
        m_domainManager.initialize();
        m_domainManager.replicateDomainConfig();
        m_dialPlanContext.replicateAutoAttendants();
        m_dialPlanContext.activateDialPlan();
        m_coreContext.initializeSpecialUsers();
        m_alarmContext.replicateAlarmServer();

        List restartable = m_processContext.getRestartable();
        m_processContext.restartOnEvent(restartable, DialPlanActivatedEvent.class);
    }

    public void onApplicationEvent(ApplicationEvent event) {
        if (!(event instanceof ApplicationInitializedEvent)) {
            // only interested in init events
            return;
        }
        if (m_adminContext.inUpgradePhase()) {
            LOG.debug("In upgrade phase - skipping initialization");
            // only process this task during normal phase
            return;
        }
        if (checkForTask()) {
            LOG.info("Handling task " + m_taskName);
            runTask();
            removeTask();
        }

    }

    private void removeTask() {
        m_adminContext.deleteInitializationTask(m_taskName);
    }

    private boolean checkForTask() {
        String[] tasks = m_adminContext.getInitializationTasks();
        return ArrayUtils.contains(tasks, m_taskName);
    }

    public void setTaskName(String taskName) {
        m_taskName = taskName;
    }

    public void setAdminContext(AdminContext adminContext) {
        m_adminContext = adminContext;
    }

    @Required
    public void setDomainManager(DomainManager domainManager) {
        m_domainManager = domainManager;
    }

    @Required
    public void setDialPlanContext(DialPlanContext dialPlanContext) {
        m_dialPlanContext = dialPlanContext;
    }

    @Required
    public void setProcessContext(SipxProcessContext processContext) {
        m_processContext = processContext;
    }

    @Required
    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    @Required
    public void setAlarmContext(AlarmContext alarmContext) {
        m_alarmContext = alarmContext;
    }    
}
