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

import java.util.Collection;
import java.util.List;

import static java.util.Collections.singleton;

import org.sipfoundry.sipxconfig.admin.ConfigurationFile;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationStatus;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
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

    private LocationsManager m_locationsManager;

    private SipxServiceManager m_sipxServiceManager;

    public void startService(Location location, SipxService service) {
        replicateServiceConfig(location, service);
        m_sipxProcessContext.manageServices(location, singleton(service), START);
    }

    /**
     * Replicates the configuration for the service and sets configuration stamp once the
     * replication succeeds.
     */
    public void replicateServiceConfig(Location location, SipxService service) {
        if (!location.isRegistered()) {
            return;
        }
        boolean serviceRequiresRestart = false;
        List< ? extends ConfigurationFile> configurations = service.getConfigurations();
        for (ConfigurationFile configuration : configurations) {
            m_replicationContext.replicate(location, configuration);
            if (configuration.isRestartRequired()) {
                serviceRequiresRestart = true;
            }
        }
        m_configVersionManager.setConfigVersion(service, location);
        if (serviceRequiresRestart) {
            m_sipxProcessContext.markServicesForRestart(singleton(service));
        }
    }

    public void replicateServiceConfig(Collection<SipxService> services) {
        for (SipxService service : services) {
            replicateServiceConfig(service);
        }
    }

    public void replicateServiceConfig(SipxService service) {
        replicateServiceConfig(service, false);
    }

    /**
     * Same as replicateServiceConfig but allows to limit replication only for configuration files
     * that do not require service restart.
     */
    public void replicateServiceConfig(SipxService service, boolean noRestartOnly) {
        List< ? extends ConfigurationFile> configurations = service.getConfigurations(noRestartOnly);
        replicateServiceConfig(service, configurations);
    }

    public void replicateLocation(Location location) {
        if (!location.isRegistered()) {
            return;
        }

        // supervisor is always installed, never on the list of standard services
        SipxService supervisorService = m_sipxServiceManager.getServiceByBeanId(SipxSupervisorService.BEAN_ID);
        replicateServiceConfig(location, supervisorService);

        for (LocationSpecificService service : location.getServices()) {
            replicateServiceConfig(location, service.getSipxService());
        }
    }

    /**
     * To replicate all services' configurations for all registered locations, and mark all
     * affected services as "Restart required".
     */
    public void replicateAllServiceConfig() {
        Location[] locations = m_locationsManager.getLocations();
        if (locations.length > 0) {
            // HACK: push dataSets and files that are not the part of normal service replication
            initLocations();
        }
        for (Location location : locations) {
            replicateLocation(location);
        }
    }

    private void replicateServiceConfig(SipxService service, Collection< ? extends ConfigurationFile> configurations) {
        boolean serviceRequiresRestart = false;
        for (ConfigurationFile configuration : configurations) {
            m_replicationContext.replicate(configuration);
            if (configuration.isRestartRequired()) {
                serviceRequiresRestart = true;
            }
        }
        if (serviceRequiresRestart) {
            m_sipxProcessContext.markServicesForRestart(singleton(service));
        }
    }

    /**
     * Stops services that need to be stopped, replicates the configuration for the ones that need
     * to be started, set configuration stamps for newly replicated configuration and finally
     * starts the services. Also restarts the services that need to be restarted as a result of
     * profile generation.
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
        // some services will need to be restarted as a result of profile generation
        m_sipxProcessContext.restartMarkedServices(location);
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
     * Needs to be called whenever we initialize or re-initialize location. It replicates data
     * sets in the same thread as the one used for pushing configuration files. It ensures the all
     * the resources are replicated before sipXconfig attempts to start the service.
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

    @Required
    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }

    @Required
    public void setSipxServiceManager(SipxServiceManager sipxServiceManager) {
        m_sipxServiceManager = sipxServiceManager;
    }
}
