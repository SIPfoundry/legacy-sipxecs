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

import org.apache.commons.collections.Factory;
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
import org.sipfoundry.sipxconfig.xmlrpc.ApiProvider;
import org.sipfoundry.sipxconfig.xmlrpc.XmlRpcRemoteException;
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

    private String m_host;

    public SipxService getServiceByBeanId(String beanId) {
        String query = QUERY_BY_BEAN_ID;
        Collection<SipxService> services = getHibernateTemplate().findByNamedQueryAndNamedParam(query, "beanId",
                beanId);

        // this is to handle a problem in unit tests where beans retrieved from
        // hibernate
        // do not have their spring attributes set
        for (SipxService sipxService : services) {
            ensureBeanIsInitialized(sipxService);
        }

        return DaoUtils.requireOneOrZero(services, query);
    }

    public Collection<SipxService> getAllServices() {
        return getHibernateTemplate().loadAll(SipxService.class);
    }

    public Map<SipxServiceBundle, List<SipxService>> getBundles() {
        Factory listFactory = new Factory() {
            public Object create() {
                return new ArrayList<SipxService>();
            }
        };
        Map<SipxServiceBundle, List<SipxService>> rawBundles = new HashMap<SipxServiceBundle, List<SipxService>>();
        Map<SipxServiceBundle, List<SipxService>> bundles = LazyMap.decorate(rawBundles, listFactory);
        Collection<SipxService> allServices = getAllServices();
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

    public void setSipxReplicationContext(SipxReplicationContext replicationContext) {
        m_replicationContext = replicationContext;
    }

    public void setApplicationContext(ApplicationContext applicationContext) {
        m_applicationContext = applicationContext;
    }

    public String getHost() {
        return m_host;
    }

    public void setHost(String host) {
        m_host = host;
    }

    public LocationsManager getLocationsManager() {
        return m_locationsManager;
    }

    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }

    public ApiProvider<ProcessManagerApi> getProcessManagerApiProvider() {
        return m_processManagerApiProvider;
    }

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
                api.setConfigVersion(m_host, service.getProcessName().getName(), version);
            } catch (XmlRpcRemoteException e) {
                LOG.error(e);
            }
        }
    }

    public void onApplicationEvent(ApplicationEvent event) {
        if (event instanceof ReplicationsFinishedEvent) {
            Collection<SipxService> services = getAllServices();
            for (SipxService service : services) {
                setConfigurationVersion(service);
            }
        }
    }
}
