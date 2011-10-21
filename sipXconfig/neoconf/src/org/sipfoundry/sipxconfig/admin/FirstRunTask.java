/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.admin;

import org.apache.commons.lang.ArrayUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.admin.alarm.AlarmServerManager;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.Location.State;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.admin.dialplan.ResetDialPlanTask;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.SbcManager;
import org.sipfoundry.sipxconfig.admin.logging.AuditLogContext;
import org.sipfoundry.sipxconfig.common.ApplicationInitializedEvent;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.paging.PagingContext;
import org.sipfoundry.sipxconfig.service.ServiceConfigurator;
import org.sipfoundry.sipxconfig.service.SipxFreeswitchService;
import org.sipfoundry.sipxconfig.service.SipxService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;
import org.springframework.beans.factory.annotation.Required;
import org.springframework.context.ApplicationEvent;
import org.springframework.context.ApplicationListener;

public class FirstRunTask implements ApplicationListener {
    private static final Log LOG = LogFactory.getLog(FirstRunTask.class);

    private CoreContext m_coreContext;
    private AdminContext m_adminContext;
    private DomainManager m_domainManager;
    private ResetDialPlanTask m_resetDialPlanTask;
    private SipxServiceManager m_sipxServiceManager;
    private ServiceConfigurator m_serviceConfigurator;
    private String m_taskName;
    private LocationsManager m_locationsManager;
    private PagingContext m_pagingContext;
    private SbcManager m_sbcManager;
    private AlarmServerManager m_alarmServerManager;
    private AuditLogContext m_auditLogContext;

    public void runTask() {
        LOG.info("Executing first run tasks...");
        m_domainManager.initializeDomain();
        m_coreContext.initializeSpecialUsers();

        // create paging server, default dial plan, default Sbc and alarm stuff
        // all of these need to exist before we start replication
        m_pagingContext.getPagingServer();
        m_resetDialPlanTask.reset(false);
        m_sbcManager.loadDefaultSbc();
        m_alarmServerManager.deployAlarms();

        //by default, select G729 codec and replicate corresponding files
        SipxService service = m_sipxServiceManager.getServiceByBeanId(SipxFreeswitchService.BEAN_ID);
        service.onInit();
        m_serviceConfigurator.replicateServiceConfig(service);

        enforceRoles();
    }

    /**
     * Make sure that the proper set of services is running for each server.
     */
    private void enforceRoles() {
        Location[] locations = m_locationsManager.getLocations();
        m_serviceConfigurator.initLocations();
        for (Location location : locations) {
            if (!location.isRegistered()) {
                continue;
            }
            location.initBundles(m_sipxServiceManager);
            location.resetBundles(m_sipxServiceManager);
            m_locationsManager.saveLocation(location);
            m_serviceConfigurator.enforceRole(location);
        }
    }

    @Override
    public void onApplicationEvent(ApplicationEvent event) {
        if (!(event instanceof ApplicationInitializedEvent)) {
            // only interested in init events
            return;
        }
        if (m_adminContext.inInitializationPhase()) {
            LOG.debug("In upgrade phase - skipping initialization");
            // only process this task during normal phase
            return;
        }
        if (checkForTask()) {
            LOG.info("Handling task " + m_taskName);
            runTask();
            removeTask();
        } else {
            // System was previously initialized, so no replication takes place
            // in this phase
            // Therefore we need to save latest replication result
            Location[] locations = m_locationsManager.getLocations();
            for (Location location : locations) {
                if (location.isInProgressState()) {
                    // it was a disaster situation and system went down during
                    // sent profiles
                    location.setState(State.NOT_FINISHED);
                    m_locationsManager.updateLocation(location);
                }
                m_auditLogContext.saveLatestReplicationsState(location);
            }
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
    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    @Required
    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }

    @Required
    public void setSipxServiceManager(SipxServiceManager sipxServiceManager) {
        m_sipxServiceManager = sipxServiceManager;
    }

    @Required
    public void setServiceConfigurator(ServiceConfigurator serviceConfigurator) {
        m_serviceConfigurator = serviceConfigurator;
    }

    @Required
    public void setPagingContext(PagingContext pagingContext) {
        m_pagingContext = pagingContext;
    }

    @Required
    public void setResetDialPlanTask(ResetDialPlanTask resetDialPlanTask) {
        m_resetDialPlanTask = resetDialPlanTask;
    }

    @Required
    public void setSbcManager(SbcManager sbcManager) {
        m_sbcManager = sbcManager;
    }

    @Required
    public void setAlarmServerManager(AlarmServerManager alarmServerManager) {
        m_alarmServerManager = alarmServerManager;
    }

    @Required
    public void setAuditLogContext(AuditLogContext auditLogContext) {
        m_auditLogContext = auditLogContext;
    }
}
