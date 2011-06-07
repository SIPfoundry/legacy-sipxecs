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

import java.sql.Timestamp;
import java.util.ArrayList;
import java.util.Calendar;
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
import org.sipfoundry.sipxconfig.acd.stats.AcdHistoricalConfigurationFile;
import org.sipfoundry.sipxconfig.admin.ConfigurationFile;
import org.sipfoundry.sipxconfig.admin.commserver.DnsGenerator;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.Location.State;
import org.sipfoundry.sipxconfig.admin.commserver.LocationStatus;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessContext;
import org.sipfoundry.sipxconfig.admin.commserver.SipxReplicationContext;
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanActivationManager;
import org.sipfoundry.sipxconfig.admin.logging.AuditLogContext;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.springframework.beans.factory.annotation.Required;
import org.springframework.context.ApplicationContext;
import org.springframework.context.ApplicationContextAware;

import static org.sipfoundry.sipxconfig.admin.commserver.SipxProcessContext.Command.START;
import static org.sipfoundry.sipxconfig.admin.commserver.SipxProcessContext.Command.STOP;

/**
 *
 */
public class ServiceConfiguratorImpl implements ServiceConfigurator, ApplicationContextAware {
    private static final Log LOG = LogFactory.getLog(ServiceConfiguratorImpl.class);
    private SipxReplicationContext m_replicationContext;
    private SipxProcessContext m_sipxProcessContext;
    private ConfigVersionManager m_configVersionManager;
    private DialPlanActivationManager m_dialPlanActivationManager;
    private LocationsManager m_locationsManager;
    private SipxServiceManager m_sipxServiceManager;
    private DnsGenerator m_dnsGenerator;
    private DomainManager m_domainManager;
    private ApplicationContext m_applicationContext;
    private AuditLogContext m_auditLogContext;
    private AcdHistoricalConfigurationFile m_acdHistoricalConfiguration;

    @Override
    public void startService(Location location, SipxService service) {
        replicateServiceConfig(location, service);
        m_sipxProcessContext.manageServices(location, singleton(service), START);
    }

    /**
     * Replicates the configuration for the service and sets configuration stamp once the
     * replication succeeds.
     */
    @Override
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

    @Override
    public void replicateServiceConfig(Collection<SipxService> services) {
        for (SipxService service : services) {
            replicateServiceConfig(service);
        }
    }

    @Override
    public void replicateServiceConfig(SipxService service) {
        // Want to replicate only config files which don't
        // require restart?
        // Nope, we want to replicate all config files.
        // Therefore, set the noRestartOnly
        // boolean to "false"
        replicateServiceConfig(service, false);
    }

    /**
     * Same as replicateServiceConfig but allows to limit replication only for configuration files
     * that do not require service restart.
     */
    @Override
    public void replicateServiceConfig(SipxService service, boolean noRestartOnly) {
        List< ? extends ConfigurationFile> configurations = service.getConfigurations(noRestartOnly);
        replicateServiceConfig(service, configurations);
    }

    @Override
    public void replicateServiceConfig(Location location, SipxService service, boolean noRestartOnly) {
        replicateServiceConfig(location, service, noRestartOnly, true);
    }

    @Override
    public void replicateServiceConfig(Location location, SipxService service, boolean noRestartOnly,
            boolean notifyService) {
        List< ? extends ConfigurationFile> configurations = service.getConfigurations(noRestartOnly);
        boolean restartRequired = replicateConfigurations(location, configurations);
        if (restartRequired) {
            m_sipxProcessContext.markServicesForRestart(singleton(service));
        }
        if (notifyService) {
            service.afterReplication(location);
        }
    }

    @Override
    public void replicateLocation(Location location) {
        if (!location.isRegistered()) {
            return;
        }
        for (LocationSpecificService service : location.getServices()) {
            replicateServiceConfig(location, service.getSipxService());
        }
        if (!location.isPrimary()) {
            m_replicationContext.resyncSlave(location);
        }
    }

    /**
     * To replicate all services' configurations for all registered locations, and mark all
     * affected services as "Restart required".
     */
    @Override
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
                LOG.info("replicate service " + service.getBeanId() + " config " + configuration.getName()
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
    @Override
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

        @Override
        public Void call() throws InterruptedException {
            replicateServiceConfig(m_serviceLocation, m_serviceToReplicate);
            return null;
        }
    }

    @Override
    public void initLocations() {
        Location[] locations = m_locationsManager.getLocations();
        if (locations.length == 0) {
            // nothing to do
            return;
        }
        for (Location location : locations) {
            initLocation(location);
        }

        replicateDialPlans();
    }

    @Override
    public void initLocation(Location location) {
        if (!location.isRegistered()) {
            return;
        }
        LOG.debug("Initializing location: " + location.getFqdn());
        LOG.debug("Replicating domain config");
        m_domainManager.replicateDomainConfig(m_replicationContext, location);
        // supervisor is always installed, never on the list of standard services
        LOG.debug("Replicating supervisor");
        SipxService supervisorService = m_sipxServiceManager.getServiceByBeanId(SipxSupervisorService.BEAN_ID);
        replicateServiceConfig(location, supervisorService);

        // replicate alarm server
        LOG.debug("Replicating alarmservice");
        SipxService alarmService = m_sipxServiceManager.getServiceByBeanId(SipxAlarmService.BEAN_ID);
        replicateServiceConfig(location, alarmService);
        if (location.isPrimary()) {
            LOG.debug("Generating the entity mongo db");
            generateDataSets();
        }
    }
    /**
     * Replicates everything, equivalent with first run task when sistem is first set up
     */
    @Override
    public synchronized void sendProfiles(Collection<Location> selectedLocations) {
        for (Location locationToActivate : selectedLocations) {
            if (!locationToActivate.isRegistered()
                    || locationToActivate.isInProgressState()) {
                continue;
            }
            // update PROGRESS state. This is useful in disaster situations
            // when we started sending profiles but operation never finishes
            locationToActivate.setState(State.PROGRESS);
            locationToActivate.setLastAttempt(new Timestamp(Calendar
                    .getInstance().getTimeInMillis()));
            // sendProfiles method replicates everything once again, so we need
            // to
            // clear current replication state
            m_auditLogContext.resetReplications(locationToActivate.getFqdn());
            // update new state without triggering other file replications
            m_locationsManager.updateLocation(locationToActivate);
            // Register location in mongo db
            m_replicationContext.replicateLocation(locationToActivate);
            // HACK: push dataSets and files that are not the part of normal
            // service replication
            initLocation(locationToActivate);

            replicateLocation(locationToActivate);
            enforceRole(locationToActivate);

            if (locationToActivate.isPrimary()) {
                m_dnsGenerator.generate();
            }
            // acdHistoricalConfiguration is a conf. file that contains data to
            // login to acd historical database used in sipxconfig-reports
            // for creating acd historical reports
            m_replicationContext.replicate(m_acdHistoricalConfiguration);
        }
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

    @Override
    public void setApplicationContext(ApplicationContext applicationContext) {
        m_applicationContext = applicationContext;
    }

    /**
     * Replicates all data sets eagerly.
     *
     * Needs to be called whenever we initialize or re-initialize location. It
     * replicates data sets in the same thread as the one used for pushing
     * configuration files. It ensures the all the resources are replicated
     * before sipXconfig attempts to start the service.
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

    @Required
    public void setDnsGenerator(DnsGenerator dnsGenerator) {
        m_dnsGenerator = dnsGenerator;
    }

    @Override
    public void markServiceForRestart(SipxService service) {
        m_sipxProcessContext.markServicesForRestart(singleton(service));
    }

    @Override
    public void markServiceForRestart(Location location, Collection< ? extends SipxService> services) {
        m_sipxProcessContext.markServicesForRestart(location, services);
    }

    @Override
    public void markServiceForRestart(Collection< ? extends SipxService> services) {
        m_sipxProcessContext.markServicesForRestart(services);
    }

    @Required
    public void setAuditLogContext(AuditLogContext auditLogContext) {
        m_auditLogContext = auditLogContext;
    }

    @Required
    public void setAcdHistoricalConfiguration(
            AcdHistoricalConfigurationFile acdHistoricalConfiguration) {
        m_acdHistoricalConfiguration = acdHistoricalConfiguration;
    }
}
