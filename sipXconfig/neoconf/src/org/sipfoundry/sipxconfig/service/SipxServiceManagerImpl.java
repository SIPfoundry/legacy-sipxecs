/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.service;

import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.apache.commons.collections.Factory;
import org.apache.commons.collections.Predicate;
import org.apache.commons.collections.Transformer;
import org.apache.commons.collections.map.LazyMap;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.sipfoundry.sipxconfig.device.ModelSource;
import org.springframework.beans.factory.annotation.Required;
import org.springframework.context.ApplicationContext;
import org.springframework.context.ApplicationContextAware;

import static org.apache.commons.collections.CollectionUtils.collect;
import static org.apache.commons.collections.CollectionUtils.filter;
import static org.apache.commons.collections.CollectionUtils.getCardinalityMap;
import static org.apache.commons.collections.CollectionUtils.intersection;

public class SipxServiceManagerImpl extends SipxHibernateDaoSupport<SipxService> implements SipxServiceManager,
        ApplicationContextAware {
    private static final Log LOG = LogFactory.getLog(SipxServiceManagerImpl.class);

    private ApplicationContext m_applicationContext;

    private LocationsManager m_locationsManager;

    private ModelSource<SipxService> m_serviceModelSource;

    private ModelSource<SipxServiceBundle> m_bundleModelSource;

    public SipxService getServiceByBeanId(String beanId) {
        Map<String, SipxService> beanId2Service = buildServiceDefinitionsMap();
        SipxService service = beanId2Service.get(beanId);
        if (service == null) {
            LOG.error("Unknown service - cannot find definition for: " + beanId);
            return null;
        }
        ensureBeanIsInitialized(service);
        return service;
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
        service.onConfigChange();
    }

    public Collection<SipxService> getServiceDefinitions() {
        Map<String, SipxService> beanIdsToServices = buildServiceDefinitionsMap();
        return beanIdsToServices.values();
    }

    /**
     * Create collection of all known services.
     *
     * If service has been added to DB already a persisted version will be returned, if it's a
     * bran new service a Spring instantiated object will be returned.
     */
    private Map<String, SipxService> buildServiceDefinitionsMap() {
        Map<String, SipxService> beanIdsToServices = new HashMap<String, SipxService>();
        for (SipxService sipxService : m_serviceModelSource.getModels()) {
            beanIdsToServices.put(sipxService.getBeanId(), sipxService);
        }
        for (SipxService sipxService : getServicesFromDb()) {
            if (sipxService.getBundles() == null) {
                // HACK: during tests bundles are not initialized properly
                // copy them from already loaded services...
                SipxService template = beanIdsToServices.get(sipxService.getBeanId());
                if (template == null) {
                    LOG.error("::buildServiceDefinitionsMap: null sipxService from model: bean: "
                            + sipxService.getBeanId());
                } else {
                    sipxService.setBundles(template.getBundles());
                }
            }
            beanIdsToServices.put(sipxService.getBeanId(), sipxService);
        }
        return beanIdsToServices;
    }

    public Collection<SipxService> getServiceDefinitions(final Collection<SipxServiceBundle> bundles) {
        Collection<SipxService> services = getServiceDefinitions();
        Predicate inBundle = new Predicate() {
            public boolean evaluate(Object item) {
                SipxService service = (SipxService) item;
                Set<SipxServiceBundle> serviceBundles = service.getBundles();
                return (serviceBundles != null && serviceBundles.size() > 0)
                        && !intersection(serviceBundles, bundles).isEmpty();
            }
        };
        filter(services, inBundle);
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
        return (List<SipxServiceBundle>) collect(installedBundles, transformer, new ArrayList());
    }

    public void setBundlesForLocation(Location location, List<SipxServiceBundle> bundles) {
        if (bundles == null || bundles.size() == 0) {
            return;
        }
        filter(bundles, new SipxServiceBundle.CanRunOn(location));
        verifyBundleCardinality(location, bundles);
        location.setInstalledBundles(transformBundlesToIds(bundles));
        location.resetBundles(this);
    }

    /**
     * Transforms the given list of bundles into a corresponding list of bundles Ids
     *
     * @param bundles affected bundles
     * @return corresponding bundles ids
     */
    private List<String> transformBundlesToIds(List<SipxServiceBundle> bundles) {
        Transformer extractModelId = new Transformer() {
            public Object transform(Object item) {
                SipxServiceBundle bundle = (SipxServiceBundle) item;
                return bundle.getModelId();
            }

        };
        return (List<String>) collect(bundles, extractModelId, new ArrayList());
    }

    /**
     * Checks if new bundle selection will honor maximum and minimum bundle counts.
     *
     * @param location server for which selection of bundles is changed
     * @param bundles proposed list of newly selected bundles for this location
     */
    void verifyBundleCardinality(Location location, List<SipxServiceBundle> bundles) {
        Location[] locations = m_locationsManager.getLocations();
        List<String> allInstalled = new ArrayList<String>();
        for (Location l : locations) {
            if (!l.equals(location)) {
                List<String> installed = l.getInstalledBundles();
                allInstalled.addAll(installed);
            }
        }
        for (SipxServiceBundle bundle : bundles) {
            allInstalled.add(bundle.getModelId());
        }
        Map<String, Integer> cardinality = getCardinalityMap(allInstalled);
        Collection<SipxServiceBundle> allBundles = m_bundleModelSource.getModels();
        for (SipxServiceBundle bundle : allBundles) {
            Integer newCount = cardinality.get(bundle.getModelId());
            if (newCount == null) {
                newCount = 0;
            }
            bundle.verifyCount(newCount);
        }
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
    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }

    public Object getServiceParam(String paramName) {
        Collection<SipxService> allServices = getServiceDefinitions();
        for (SipxService service : allServices) {
            if (!isServiceInstalled(service.getBeanId())) {
                continue;
            }
            Object param = service.getParam(paramName);
            if (param != null) {
                return param;
            }
        }
        return null;
    }
}
