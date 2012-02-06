/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.cfgmgt;

import java.io.File;
import java.io.Serializable;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.address.AddressProvider;
import org.sipfoundry.sipxconfig.address.AddressType;
import org.sipfoundry.sipxconfig.common.LazyDaemon;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.commserver.SipxReplicationContext;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.feature.Feature;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.job.JobContext;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.beans.factory.ListableBeanFactory;
import org.springframework.beans.factory.annotation.Required;

public class ConfigManagerImpl implements AddressProvider, ConfigManager, BeanFactoryAware {
    private static final Log LOG = LogFactory.getLog(ConfigManagerImpl.class);
    private File m_cfDataDir;
    private DomainManager m_domainManager;
    private FeatureManager m_featureManager;
    private AddressManager m_addressManager;
    private LocationsManager m_locationManager;
    private Collection<ConfigProvider> m_providers;
    private ListableBeanFactory m_beanFactory;
    private int m_sleepInterval = 7000;
    private ConfigWorker m_worker;
    private final ConfigRequest[] m_outstandingRequest = new ConfigRequest[1];
    private Set<Feature> m_affectedFeatures = new HashSet<Feature>();
    private boolean m_allFeaturesAffected;
    private ConfigAgent m_configAgent;
    private SipxReplicationContext m_sipxReplicationContext;
    private JobContext m_jobContext;

    public void init() {
        m_worker = new ConfigWorker();
        m_worker.start();
    }

    @Override
    public synchronized void configureEverywhere(Feature... features) {
        // (re)start timer
        m_outstandingRequest[0] = ConfigRequest.merge(ConfigRequest.only(features), m_outstandingRequest[0]);
        notifyWorker();
    }

    private void notifyWorker() {
        synchronized (m_worker) {
            m_worker.workScheduled();
            m_worker.notify();
        }
    }

    @Override
    public synchronized void configureAllFeaturesEverywhere() {
        m_allFeaturesAffected = true;
        m_outstandingRequest[0] = ConfigRequest.merge(ConfigRequest.always(), m_outstandingRequest[0]);
        notifyWorker();
    }

    @Override
    public synchronized void configureAllFeatures(Collection<Location> locations) {
        m_outstandingRequest[0] = ConfigRequest.merge(ConfigRequest.only(locations), m_outstandingRequest[0]);
        notifyWorker();
    }

    @Override
    public synchronized void regenerateMongo(Collection<Location> locations) {
        if (locations.contains(m_locationManager.getPrimaryLocation())) {
            m_sipxReplicationContext.generateAll();
        }
    }

    @Override
    public void sendProfiles(Collection<Location> locations) {
        regenerateMongo(locations);
        configureAllFeatures(locations);
    }

    public synchronized boolean hasWork() {
        return m_allFeaturesAffected || !m_affectedFeatures.isEmpty();
    }

    public synchronized ConfigRequest getWork() {
        ConfigRequest work = m_outstandingRequest[0];
        m_outstandingRequest[0] = null;
        return work;
    }

    // not synchronized so new incoming work can accumulate.
    public void doWork(ConfigRequest request) {
        LOG.info("Configuration work to do. Notifying providers.");
        Serializable job = m_jobContext.schedule("Configuration");
        m_jobContext.start(job);
        List<Exception> errors = new ArrayList<Exception>();
        for (ConfigProvider provider : getProviders()) {
            try {
                provider.replicate(this, request);
            } catch (Exception e) {
                errors.add(e);
            }
        }
        try {
            m_configAgent.run();
        } catch (ConfigException e) {
            errors.add(e);
        }
        if (errors.size() > 0) {
            m_jobContext.failure(job, getErrorMessage(errors), new RuntimeException());
        } else {
            m_jobContext.success(job);
        }
    }

    String getErrorMessage(List<Exception> errors) {
        StringBuilder msg = new StringBuilder();
        for (Exception err : errors) {
            if (msg.length() != 0) {
                msg.append("\n");
            }
            if (err instanceof ConfigException) {
                msg.append(err.getMessage());
            } else {
                msg.append("Internal error (" + err + ")");
            }
            LOG.error("Configuration Error", err);
        }
        return msg.toString();
    }

    public String getCfDataDir() {
        return m_cfDataDir.getAbsolutePath();
    }

    public void setCfDataDir(String cfDataDir) {
        m_cfDataDir = new File(cfDataDir);
    }

    @Override
    public File getGlobalDataDirectory() {
        return m_cfDataDir;
    }

    @Override
    public File getLocationDataDirectory(Location location) {
        File d = new File(m_cfDataDir, String.valueOf(location.getId()));
        if (!d.exists()) {
            d.mkdirs();
        }
        return d;
    }

    @Override
    public ConfigStatus getStatus(Location location, String key) {
        return ConfigStatus.OK;
    }

    @Override
    public void restartAllJavaProcesses() {
        // TODO
    }

    @Override
    public void restartService(Location location, String service) {
        // TODO
    }

    @Override
    public DomainManager getDomainManager() {
        return m_domainManager;
    }

    @Required
    public void setDomainManager(DomainManager domainManager) {
        m_domainManager = domainManager;
    }

    @Override
    public FeatureManager getFeatureManager() {
        return m_featureManager;
    }

    public void setFeatureManager(FeatureManager featureManager) {
        m_featureManager = featureManager;
    }

    @Override
    public AddressManager getAddressManager() {
        return m_addressManager;
    }

    public void setAddressManager(AddressManager addressManager) {
        m_addressManager = addressManager;
    }

    @Override
    public LocationsManager getLocationManager() {
        return m_locationManager;
    }

    public void setLocationManager(LocationsManager locationManager) {
        m_locationManager = locationManager;
    }

    @Override
    public Collection<AddressType> getSupportedAddressTypes(AddressManager manager) {
        return Collections.singleton(SUPERVISOR_ADDRESS);
    }

    @Override
    public Collection<Address> getAvailableAddresses(AddressManager manager, AddressType type, Object requester) {
        if (type.equals(SUPERVISOR_ADDRESS)) {
            // this will eventually phase out in favor of sipxsupervisor-lite
            return Collections.singleton(new Address(null, 8092));
        }
        return null;
    }

    private Collection<ConfigProvider> getProviders() {
        if (m_providers == null) {
            Map<String, ConfigProvider> providers = m_beanFactory.getBeansOfType(ConfigProvider.class, false, false);
            m_providers = providers.values();
        }
        return m_providers;
    }

    @Override
    public void setBeanFactory(BeanFactory beanFactory) {
        m_beanFactory = (ListableBeanFactory) beanFactory;
    }

    class ConfigWorker extends LazyDaemon {
        ConfigWorker() {
            super("Replication worker thread", m_sleepInterval);
        }

        @Override
        protected void waitForWork() throws InterruptedException {
            if (!hasWork()) {
                synchronized (ConfigWorker.this) {
                    ConfigWorker.this.wait();
                }
            }
        }

        @Override
        protected boolean work() {
            ConfigRequest work = getWork();
            doWork(work);
            return true;
        }
    }

    public void setConfigAgent(ConfigAgent configAgent) {
        m_configAgent = configAgent;
    }

    public void setSipxReplicationContext(SipxReplicationContext sipxReplicationContext) {
        m_sipxReplicationContext = sipxReplicationContext;
    }

    public void setJobContext(JobContext jobContext) {
        m_jobContext = jobContext;
    }
}
