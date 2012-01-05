/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.cfgmgt;

import java.io.File;
import java.io.IOException;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.HashSet;
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
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.feature.Feature;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
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
    private Set<Feature> m_affectedFeatures = new HashSet<Feature>();
    private boolean m_allFeaturesAffected;

    public void init() {
        m_worker = new ConfigWorker();
        m_worker.start();
    }

    @Override
    public synchronized void replicationRequired(Feature feature) {
        // (re)start timer
        m_affectedFeatures.add(feature);
        synchronized (m_worker) {
            m_worker.notify();
        }
    }

    @Override
    public synchronized void replicationRequired(Feature ... features) {
        // (re)start timer
        m_affectedFeatures.addAll(Arrays.asList(features));
        synchronized (m_worker) {
            m_worker.notify();
        }
    }

    public synchronized boolean hasWork() {
        return m_allFeaturesAffected || !m_affectedFeatures.isEmpty();
    }

    public synchronized ConfigRequest getWork() {
        ConfigRequest request;
        if (m_allFeaturesAffected) {
            request = ConfigRequest.always();
        } else {
            HashSet<Feature> affected = new HashSet<Feature>(m_affectedFeatures);
            request = ConfigRequest.only(affected);
        }
        m_allFeaturesAffected = false;
        m_affectedFeatures.clear();
        return request;
    }

    // not synchronized so new incoming work can accumulate.
    public void doWork(ConfigRequest request) {
        LOG.info("Configuration work to do. Notifying providers. ");
        for (ConfigProvider provider : getProviders()) {
            try {
                provider.replicate(this, request);
            } catch (IOException e) {
                // TODO Log failure somewhere important but do not abort loop
                e.printStackTrace();
            }
        }
        runAgent();
    }

    void runAgent() {
    }

    @Override
    public synchronized void allFeaturesAffected() {
        m_allFeaturesAffected = true;
        synchronized (m_worker) {
            m_worker.notify();
        }
    }

    public String getCfDataDir() {
        return m_cfDataDir.getAbsolutePath();
    }

    public void setCfDataDir(String cfDataDir) {
        m_cfDataDir = new File(cfDataDir);
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
            Map<String, ConfigProvider> providers = m_beanFactory.getBeansOfType(ConfigProvider.class);
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
}
