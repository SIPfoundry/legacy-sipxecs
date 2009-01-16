/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.service;

import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.collections.Factory;
import org.apache.commons.collections.Predicate;
import org.apache.commons.collections.Transformer;
import org.apache.commons.collections.map.LazyMap;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.admin.commserver.ProcessManagerApi;
import org.sipfoundry.sipxconfig.admin.commserver.SipxReplicationContext;
import org.sipfoundry.sipxconfig.common.DaoUtils;
import org.sipfoundry.sipxconfig.common.ReplicationsFinishedEvent;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.sipfoundry.sipxconfig.common.VersionInfo;
import org.sipfoundry.sipxconfig.device.ModelSource;
import org.sipfoundry.sipxconfig.xmlrpc.ApiProvider;
import org.sipfoundry.sipxconfig.xmlrpc.XmlRpcRemoteException;
import org.springframework.beans.factory.annotation.Required;
import org.springframework.context.ApplicationContext;
import org.springframework.context.ApplicationContextAware;
import org.springframework.context.ApplicationEvent;
import org.springframework.context.ApplicationListener;

public class SipxServiceManagerImpl extends SipxHibernateDaoSupport<SipxService> implements SipxServiceManager,
        ApplicationContextAware, ApplicationListener {

    private static final String QUERY_BY_BEAN_ID = "service-by-bean-id";

    private static final Log LOG = LogFactory.getLog(SipxServiceManagerImpl.class);

    private SipxReplicationContext m_replicationContext;

    private ApplicationContext m_applicationContext;

    private LocationsManager m_locationsManager;

    private ApiProvider<ProcessManagerApi> m_processManagerApiProvider;

    private ModelSource<SipxService> m_serviceModelSource;

    private ModelSource<SipxServiceBundle> m_bundleModelSource;

    private String m_host;

    public SipxService getServiceByBeanId(String beanId) {
        String query = QUERY_BY_BEAN_ID;
        Collection<SipxService> services = getHibernateTemplate().findByNamedQueryAndNamedParam(query, "beanId",
                beanId);

        for (SipxService sipxService : services) {
            ensureBeanIsInitialized(sipxService);
        }

        return DaoUtils.requireOneOrZero(services, query);
    }

    public SipxService getServiceByName(String name) {
        Collection<SipxService> allServices = getServiceDefinitions();
        for (SipxService service : allServices) {
            if (name.equals(service.getProcessName())) {
                return service;
            }
        }
        return null;
    }

    public List<SipxService> getRestartable() {
        Collection<SipxService> allServices = getServiceDefinitions();
        List<SipxService> restartable = new ArrayList<SipxService>(allServices.size());
        for (SipxService sipxService : allServices) {
            if (sipxService.isRestartable()) {
                restartable.add(sipxService);
            }
        }
        return restartable;
    }

    Collection<SipxService> getServicesFromDb() {
        return getHibernateTemplate().loadAll(SipxService.class);
    }

    public boolean isServiceInstalled(Integer locationId, String serviceBeanId) {
        Location location = m_locationsManager.getLocation(locationId);
        Collection<LocationSpecificService> services = location.getServices();
        for (LocationSpecificService service : services) {
            SipxService sipxService = service.getSipxService();
            if (sipxService.getBeanId().equals(serviceBeanId)) {
                return true;
            }
        }
        return false;
    }

    public boolean isServiceInstalled(String serviceBeanId) {
        Location[] locations = m_locationsManager.getLocations();
        for (Location location : locations) {
            if (isServiceInstalled(location.getId(), serviceBeanId)) {
                return true;
            }
        }
        return false;
    }

    public Map<SipxServiceBundle, List<SipxService>> getBundles() {
        Factory listFactory = new Factory() {
            public Object create() {
                return new ArrayList<SipxService>();
            }
        };
        Map<SipxServiceBundle, List<SipxService>> rawBundles = new HashMap<SipxServiceBundle, List<SipxService>>();
        Map<SipxServiceBundle, List<SipxService>> bundles = LazyMap.decorate(rawBundles, listFactory);
        Collection<SipxService> allServices = getServiceDefinitions();
        for (SipxService service : allServices) {
            Set<SipxServiceBundle> serviceBundles = service.getBundles();
            if (serviceBundles == null) {
                LOG.warn(service.getBeanId() + " does not belong to any bundle");
                continue;
            }
            for (SipxServiceBundle bundle : serviceBundles) {
                bundles.get(bundle).add(service);
            }
        }
        return rawBundles;
    }

    /**
     * This method handles problems in unit tests where beans retrieved from hibernate do not have
     * their spring attributes set.
     */
    private void ensureBeanIsInitialized(SipxService sipxService) {
        if (sipxService.getModelFilesContext() == null) {
            String beanId = sipxService.getBeanId();
            SipxService serviceTemplate = (SipxService) m_applicationContext.getBean(beanId);
            sipxService.setModelFilesContext(serviceTemplate.getModelFilesContext());
            sipxService.setModelDir(serviceTemplate.getModelDir());
            sipxService.setModelName(serviceTemplate.getModelName());
        }
    }

    public void storeService(SipxService service) {
        saveBeanWithSettings(service);
        replicateServiceConfig(service);
    }

    public void replicateServiceConfig(SipxService service) {
        List<SipxServiceConfiguration> configurations = service.getConfigurations();
        if (configurations.size() < 1) {
            LOG.warn("Unable to replicate service: " + service.getBeanId() + ". No configuration objects defined.");
            return;
        }
        for (SipxServiceConfiguration configuration : configurations) {
            m_replicationContext.replicate(configuration);
        }
    }

    @Required
    public void setSipxReplicationContext(SipxReplicationContext replicationContext) {
        m_replicationContext = replicationContext;
    }

    public void setApplicationContext(ApplicationContext applicationContext) {
        m_applicationContext = applicationContext;
    }

    @Required
    public void setServiceModelSource(ModelSource<SipxService> serviceModelSource) {
        m_serviceModelSource = serviceModelSource;
    }

    @Required
    public void setBundleModelSource(ModelSource<SipxServiceBundle> bundleModelSource) {
        m_bundleModelSource = bundleModelSource;
    }

    @Required
    public void setHost(String host) {
        m_host = host;
    }

    @Required
    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }

    @Required
    public void setProcessManagerApiProvider(ApiProvider<ProcessManagerApi> processManagerApiProvider) {
        m_processManagerApiProvider = processManagerApiProvider;
    }

    public void setConfigurationVersion(SipxService service) {
        Location[] locations = m_locationsManager.getLocations();
        for (int i = 0; i < locations.length; i++) {
            Location location = locations[i];
            ProcessManagerApi api = m_processManagerApiProvider.getApi(location.getProcessMonitorUrl());
            VersionInfo versionInfo = new VersionInfo();
            String version = versionInfo.getVersion();
            try {
                api.setConfigVersion(m_host, service.getProcessName(), version);
            } catch (XmlRpcRemoteException e) {
                LOG.error(e);
            }
        }
    }

    public void onApplicationEvent(ApplicationEvent event) {
        if (event instanceof ReplicationsFinishedEvent) {
            Collection<SipxService> services = getServicesFromDb();
            for (SipxService service : services) {
                setConfigurationVersion(service);
            }
        }
    }

    /**
     * Create collection of all known services.
     *
     * If service has been added to DB already a persisted version will be returned, if it's a
     * bran new service a Spring instantiated object will be returned.
     */
    public Collection<SipxService> getServiceDefinitions() {
        Map<String, SipxService> beanIdsToServices = new HashMap<String, SipxService>();
        for (SipxService sipxService : m_serviceModelSource.getModels()) {
            beanIdsToServices.put(sipxService.getBeanId(), sipxService);
        }
        for (SipxService sipxService : getServicesFromDb()) {
            beanIdsToServices.put(sipxService.getBeanId(), sipxService);
        }
        return beanIdsToServices.values();
    }

    /**
     * Create collection of the services that belong to the specific subset of bundles
     */
    public Collection<SipxService> getServiceDefinitions(final Collection<SipxServiceBundle> bundles) {
        Collection<SipxService> services = getServiceDefinitions();
        Predicate inBundle = new Predicate() {
            public boolean evaluate(Object item) {
                SipxService service = (SipxService) item;
                Set<SipxServiceBundle> serviceBundles = service.getBundles();
                return !CollectionUtils.intersection(serviceBundles, bundles).isEmpty();
            }
        };
        CollectionUtils.filter(services, inBundle);
        return services;
    }

    public Collection<SipxServiceBundle> getBundleDefinitions() {
        return m_bundleModelSource.getModels();
    }

    public SipxServiceBundle getBundleByName(String name) {
        Collection<SipxServiceBundle> bundles = m_bundleModelSource.getModels();
        for (SipxServiceBundle bundle : bundles) {
            if (name.equals(bundle.getName())) {
                return bundle;
            }
        }
        return null;
    }

    public List<SipxServiceBundle> getBundlesForLocation(Location location) {
        Transformer transformer = new Transformer() {
            public Object transform(Object modelId) {
                return m_bundleModelSource.getModel((String) modelId);
            }
        };
        List<String> installedBundles = location.getInstalledBundles();
        return (List<SipxServiceBundle>) CollectionUtils.collect(installedBundles, transformer, new ArrayList());
    }

    public void setBundlesForLocation(Location location, List<SipxServiceBundle> bundles) {
        Collection<SipxService> oldServices = location.getSipxServices();
        Collection<SipxService> newServices = getServiceDefinitions(bundles);

        Collection<SipxService> stopServices = CollectionUtils.subtract(oldServices, newServices);
        Collection<SipxService> startServices = CollectionUtils.subtract(newServices, oldServices);

        location.removeServices(stopServices);
        location.addServices(startServices);

        Transformer extractModelId = new Transformer() {
            public Object transform(Object item) {
                SipxServiceBundle bundle = (SipxServiceBundle) item;
                return bundle.getModelId();
            }

        };
        List<String> bundleIds = (List<String>) CollectionUtils.collect(bundles, extractModelId, new ArrayList());
        location.setInstalledBundles(bundleIds);
    }
}
