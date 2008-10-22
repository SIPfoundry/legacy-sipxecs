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

import java.util.Collection;
import java.util.List;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.admin.commserver.SipxReplicationContext;
import org.sipfoundry.sipxconfig.common.DaoUtils;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.springframework.context.ApplicationContext;
import org.springframework.context.ApplicationContextAware;

public class SipxServiceManagerImpl extends SipxHibernateDaoSupport<SipxService> implements
        SipxServiceManager, ApplicationContextAware {

    private static final String QUERY_BY_BEAN_ID = "service-by-bean-id";
    private static final Log LOG = LogFactory.getLog(SipxServiceManagerImpl.class);
    private SipxReplicationContext m_replicationContext;
    private ApplicationContext m_applicationContext;

    public SipxService getServiceByBeanId(String beanId) {
        String query = QUERY_BY_BEAN_ID;
        Collection<SipxService> services = getHibernateTemplate().findByNamedQueryAndNamedParam(
                query, "beanId", beanId);

        // this is to handle a problem in unit tests where beans retrieved from hibernate
        // do not have their spring attributes set
        for (SipxService sipxService : services) {
            ensureBeanIsInitialized(sipxService);
        }

        return DaoUtils.requireOneOrZero(services, query);
    }

    public Collection<SipxService> getAllServices() {
        return getHibernateTemplate().loadAll(SipxService.class);
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
            LOG.warn("Unable to replicate service: " + service.getBeanId()
                    + ". No configuration objects defined.");
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
}
