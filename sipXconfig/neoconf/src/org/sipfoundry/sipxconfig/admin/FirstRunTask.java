/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.admin;

import java.util.Collection;

import org.apache.commons.lang.ArrayUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanActivationManager;
import org.sipfoundry.sipxconfig.admin.parkorbit.ParkOrbitContext;
import org.sipfoundry.sipxconfig.common.AlarmContext;
import org.sipfoundry.sipxconfig.common.ApplicationInitializedEvent;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.device.ProfileManager;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.gateway.GatewayContext;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.service.ServiceConfigurator;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;
import org.sipfoundry.sipxconfig.speeddial.SpeedDialManager;
import org.springframework.beans.factory.annotation.Required;
import org.springframework.context.ApplicationEvent;
import org.springframework.context.ApplicationListener;

public class FirstRunTask implements ApplicationListener {
    private static final Log LOG = LogFactory.getLog(FirstRunTask.class);

    private CoreContext m_coreContext;
    private AdminContext m_adminContext;
    private DomainManager m_domainManager;
    private DialPlanActivationManager m_dialPlanActivationManager;
    private SipxServiceManager m_sipxServiceManager;
    private ServiceConfigurator m_serviceConfigurator;
    private String m_taskName;
    private AlarmContext m_alarmContext;
    private GatewayContext m_gatewayContext;
    private PhoneContext m_phoneContext;
    private ProfileManager m_gatewayProfileManager;
    private ProfileManager m_phoneProfileManager;
    private LocationsManager m_locationsManager;
    private ParkOrbitContext m_parkOrbitContext;
    private SpeedDialManager m_speedDialManager;

    public void runTask() {
        LOG.info("Executing first run tasks...");
        m_domainManager.initializeDomain();
        m_domainManager.replicateDomainConfig();
        m_coreContext.initializeSpecialUsers();

        // this is moved from replication trigger will need something better here...
        m_parkOrbitContext.activateParkOrbits();
        m_speedDialManager.activateResourceList();
        m_dialPlanActivationManager.replicateDialPlan(false);
        m_alarmContext.replicateAlarmServer();

        enforceRoles();
        generateAllProfiles();
    }

    /**
     * Make sure that the proper set of services is running for each server.
     */
    private void enforceRoles() {
        Location[] locations = m_locationsManager.getLocations();
        for (Location location : locations) {
            location.initBundles(m_sipxServiceManager);
            location.resetBundles(m_sipxServiceManager);
            m_locationsManager.storeLocation(location);
            m_serviceConfigurator.enforceRole(location);
        }
    }

    /**
     * Regenerate all profiles: important if profile format changed after upgrade.
     */
    private void generateAllProfiles() {
        LOG.info("Updating gateway profiles.");
        Collection gatewayIds = m_gatewayContext.getAllGatewayIds();
        m_gatewayProfileManager.generateProfiles(gatewayIds, true, null);

        LOG.info("Updating phones profiles.");
        Collection phoneIds = m_phoneContext.getAllPhoneIds();
        m_phoneProfileManager.generateProfiles(phoneIds, true, null);
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

    public void setDialPlanActivationManager(DialPlanActivationManager dialPlanActivationManager) {
        m_dialPlanActivationManager = dialPlanActivationManager;
    }

    @Required
    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    @Required
    public void setAlarmContext(AlarmContext alarmContext) {
        m_alarmContext = alarmContext;
    }

    @Required
    public void setPhoneContext(PhoneContext phoneContext) {
        m_phoneContext = phoneContext;
    }

    @Required
    public void setGatewayContext(GatewayContext gatewayContext) {
        m_gatewayContext = gatewayContext;
    }

    @Required
    public void setGatewayProfileManager(ProfileManager gatewayProfileManager) {
        m_gatewayProfileManager = gatewayProfileManager;
    }

    @Required
    public void setPhoneProfileManager(ProfileManager phoneProfileManager) {
        m_phoneProfileManager = phoneProfileManager;
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
    public void setParkOrbitContext(ParkOrbitContext parkOrbitContext) {
        m_parkOrbitContext = parkOrbitContext;
    }

    @Required
    public void setSpeedDialManager(SpeedDialManager speedDialManager) {
        m_speedDialManager = speedDialManager;
    }

    @Required
    public void setServiceConfigurator(ServiceConfigurator serviceConfigurator) {
        m_serviceConfigurator = serviceConfigurator;
    }
}
