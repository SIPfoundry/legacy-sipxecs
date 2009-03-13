/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.service;

import java.util.List;

import static java.util.Collections.singleton;

import org.sipfoundry.sipxconfig.admin.ConfigurationFile;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationStatus;
import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessContext;
import org.sipfoundry.sipxconfig.admin.commserver.SipxReplicationContext;
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanActivationManager;
import org.springframework.beans.factory.annotation.Required;

import static org.sipfoundry.sipxconfig.admin.commserver.SipxProcessContext.Command.START;
import static org.sipfoundry.sipxconfig.admin.commserver.SipxProcessContext.Command.STOP;

public class ServiceConfiguratorImpl implements ServiceConfigurator {
    private SipxReplicationContext m_replicationContext;

    private SipxProcessContext m_sipxProcessContext;

    private ConfigVersionManager m_configVersionManager;

    private DialPlanActivationManager m_dialPlanActivationManager;

    public void startService(Location location, SipxService service) {
        replicateServiceConfig(location, service);
        m_sipxProcessContext.manageServices(location, singleton(service), START);
    }

    public void replicateServiceConfig(SipxService service) {
        List< ? extends ConfigurationFile> configurations = service.getConfigurations();
        for (ConfigurationFile configuration : configurations) {
            m_replicationContext.replicate(configuration);
        }
        m_sipxProcessContext.markServicesForRestart(singleton(service));
    }

    /**
     * Replicates the configuration for the service and sets configuration stamp once the
     * replication succeeds.
     */
    public void replicateServiceConfig(Location location, SipxService service) {
        if (!location.isRegistered()) {
            return;
        }
        List< ? extends ConfigurationFile> configurations = service.getConfigurations();
        for (ConfigurationFile configuration : configurations) {
            m_replicationContext.replicate(location, configuration);
        }
        m_configVersionManager.setConfigVersion(service, location);
    }

    /**
     * Stops services that need to be stopped, replicates the configuration for the ones that need
     * to be started, set configuration stamps for newly replicated configuration and finally
     * starts the services.
     */
    public void enforceRole(Location location) {
        if (!location.isRegistered()) {
            return;
        }
        LocationStatus locationStatus = m_sipxProcessContext.getLocationStatus(location);
        m_sipxProcessContext.manageServices(location, locationStatus.getToBeStopped(), STOP);
        for (SipxService service : locationStatus.getToBeStarted()) {
            replicateServiceConfig(location, service);
        }
        m_sipxProcessContext.manageServices(location, locationStatus.getToBeStarted(), START);
    }

    public void initLocations() {
        generateDataSets();
        replicateDialPlans();
    }

    /**
     * Replicates dial plans eagerly.
     *
     * This is a temporary hack: at some point all files that comprise dial plan should be
     * declared as configuration files that belong to their owners and get replciated before
     * respective services are started.
     */
    @Deprecated
    private void replicateDialPlans() {
        m_dialPlanActivationManager.replicateDialPlan(false);
    }

    /**
     * Replicates all data sets eagerly.
     *
     * Needs to be called whenever we initilize or re-initialize location. It replicates data sets
     * in the same thread as the one used for pushing configuration files. It ensures the all the
     * resources are replicated before sipXconfig attempts to start the service.
     */
    private void generateDataSets() {
        m_replicationContext.generateAll();
    }

    @Required
    public void setSipxReplicationContext(SipxReplicationContext replicationContext) {
        m_replicationContext = replicationContext;
    }

    @Required
    public void setSipxProcessContext(SipxProcessContext sipxProcessContext) {
        m_sipxProcessContext = sipxProcessContext;
    }

    @Required
    public void setConfigVersionManager(ConfigVersionManager configVersionManager) {
        m_configVersionManager = configVersionManager;
    }

    @Required
    public void setDialPlanActivationManager(DialPlanActivationManager dialPlanActivationManager) {
        m_dialPlanActivationManager = dialPlanActivationManager;
    }
}
