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

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;
import java.util.concurrent.Callable;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;

import static java.util.Collections.singleton;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.admin.ConfigurationFile;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationStatus;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessContext;
import org.sipfoundry.sipxconfig.admin.commserver.SipxReplicationContext;
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanActivationManager;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.springframework.beans.factory.annotation.Required;

import static org.sipfoundry.sipxconfig.admin.commserver.SipxProcessContext.Command.START;
import static org.sipfoundry.sipxconfig.admin.commserver.SipxProcessContext.Command.STOP;

public class ServiceConfiguratorImpl implements ServiceConfigurator {
    private static final Log LOG = LogFactory.getLog(ServiceConfiguratorImpl.class);

    private SipxReplicationContext m_replicationContext;

    private SipxProcessContext m_sipxProcessContext;

    private ConfigVersionManager m_configVersionManager;

    private DialPlanActivationManager m_dialPlanActivationManager;

    private LocationsManager m_locationsManager;

    private SipxServiceManager m_sipxServiceManager;

    private DomainManager m_domainManager;

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
        List< ? extends ConfigurationFile> configurations = service.getConfigurations();
        boolean serviceRequiresRestart = replicateConfigurations(location, configurations);
        m_configVersionManager.setConfigVersion(service, location);
        if (serviceRequiresRestart) {
            m_sipxProcessContext.markServicesForRestart(singleton(service));
        }
        service.afterReplication(location);
    }

    public void replicateServiceConfig(Collection<SipxService> services) {
        for (SipxService service : services) {
            replicateServiceConfig(service);
        }
    }

    public void replicateServiceConfig(SipxService service) {
        //Want to replicate only config files which don't
        //require restart?
        // Nope, we want to replicate all config files.
        // Therefore, set the noRestartOnly
        //boolean to "false"
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

    public void replicateServiceConfig(Location location, SipxService service, boolean noRestartOnly) {
        List< ? extends ConfigurationFile> configurations = service.getConfigurations(noRestartOnly);
        boolean restartRequired = replicateConfigurations(location, configurations);
        if (restartRequired) {
            m_sipxProcessContext.markServicesForRestart(singleton(service));
        }
        service.afterReplication(location);
    }

    public void replicateLocation(Location location) {
        if (!location.isRegistered()) {
            return;
        }
        for (LocationSpecificService service : location.getServices()) {
            replicateServiceConfig(location, service.getSipxService());
        }
    }

    /**
     * To replicate all services' configurations for all registered locations, and mark all
     * affected services as "Restart required".
     */
    public void replicateAllServiceConfig() {
        initLocations();
        Location[] locations = m_locationsManager.getLocations();
        for (Location location : locations) {
            replicateLocation(location);
        }
    }

    private void replicateServiceConfig(SipxService service, Collection< ? extends ConfigurationFile> configurations) {
        boolean serviceRequiresRestart = false;
        for (ConfigurationFile configuration : configurations) {
            m_replicationContext.replicate(configuration);
            if (configuration.isRestartRequired()) {
                LOG.info("replicate service " + service.getBeanId()
                    + " config " + configuration.getName()
                    + " Restart required:" + configuration.isRestartRequired());
                serviceRequiresRestart = true;
            }
        }
        if (serviceRequiresRestart) {
            m_sipxProcessContext.markServicesForRestart(singleton(service));
        }
        service.afterReplication(null);
    }

    private boolean replicateConfigurations(Location location, List< ? extends ConfigurationFile> configurations) {
        boolean serviceRequiresRestart = false;
        for (ConfigurationFile configuration : configurations) {
            m_replicationContext.replicate(location, configuration);
            if (configuration.isRestartRequired()) {
                serviceRequiresRestart = true;
            }
        }
        return serviceRequiresRestart;
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
        try {
            Collection<SipxService> services = locationStatus.getToBeReplicated();
            if (!services.isEmpty()) {
                ExecutorService executorService = Executors.newFixedThreadPool(services.size());
                ArrayList<Future<Void>> futures = new ArrayList<Future<Void>>(services.size());
                for (SipxService service : services) {
                    futures.add(executorService.submit(new ReplicateServiceConfigurations(location, service)));
                }
                executorService.shutdown();

                for (Future<Void> future : futures) {
                    future.get();
                }
            }
        } catch (InterruptedException exception) {
            LOG.error("Unexpected Interupt when replicating services", exception);
        } catch (ExecutionException exception) {
            LOG.error("Unexpected error when replicating services", exception);
        }

        m_sipxProcessContext.manageServices(location, locationStatus.getToBeStarted(), START);
        // some services will need to be restarted as a result of profile generation
        m_sipxProcessContext.restartMarkedServices(location);
    }

    class ReplicateServiceConfigurations implements Callable<Void> {
        private final Location m_serviceLocation;
        private final SipxService m_serviceToReplicate;

        public ReplicateServiceConfigurations(Location serviceLocation, SipxService serviceToReplicate) {
            super();
            m_serviceLocation = serviceLocation;
            m_serviceToReplicate = serviceToReplicate;
        }

        public Void call() throws InterruptedException {
            replicateServiceConfig(m_serviceLocation, m_serviceToReplicate);
            return null;
        }
    }

    public void initLocations() {
        Location[] locations = m_locationsManager.getLocations();
        if (locations.length == 0) {
            // nothing to do
            return;
        }
        for (Location location : locations) {
            if (!location.isRegistered()) {
                continue;
            }
            m_domainManager.replicateDomainConfig(m_replicationContext, location);
            // supervisor is always installed, never on the list of standard services
            SipxService supervisorService = m_sipxServiceManager.getServiceByBeanId(SipxSupervisorService.BEAN_ID);
            replicateServiceConfig(location, supervisorService);

            // replicate alarm server
            SipxService alarmService = m_sipxServiceManager.getServiceByBeanId(SipxAlarmService.BEAN_ID);
            replicateServiceConfig(location, alarmService);
        }

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

    @Required
    public void setDomainManager(DomainManager domainManager) {
        m_domainManager = domainManager;
    }

    public void markServiceForRestart(SipxService service) {
        m_sipxProcessContext.markServicesForRestart(singleton(service));
    }

    public void markServiceForRestart(Location location, Collection< ? extends SipxService> services) {
        m_sipxProcessContext.markServicesForRestart(location, services);
    }

    public void markServiceForRestart(Collection< ? extends SipxService> services) {
        m_sipxProcessContext.markServicesForRestart(services);
    }

}
