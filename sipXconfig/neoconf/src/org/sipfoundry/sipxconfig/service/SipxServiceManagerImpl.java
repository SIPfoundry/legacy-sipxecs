/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.service;

import java.util.Collection;

import org.sipfoundry.sipxconfig.admin.commserver.SipxReplicationContext;
import org.sipfoundry.sipxconfig.common.DaoUtils;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;

public class SipxServiceManagerImpl extends SipxHibernateDaoSupport<SipxService> implements
        SipxServiceManager {

    private static final String QUERY_BY_BEAN_ID = "service-by-bean-id";
    private SipxReplicationContext m_replicationContext;

    public SipxService getServiceByBeanId(String beanId) {
        String query = QUERY_BY_BEAN_ID;
        Collection<SipxService> services = getHibernateTemplate().findByNamedQueryAndNamedParam(
                query, "beanId", beanId);
        return DaoUtils.requireOneOrZero(services, query);
    }

    public void storeService(SipxService service) {
        saveBeanWithSettings(service);
        replicateServiceConfig(service);
    }
    
    public void replicateServiceConfig(SipxService service) {
        SipxServiceConfiguration configuration = service.getConfiguration();
        configuration.generate(service);
        m_replicationContext.replicate(configuration);
    }
    
    public void setSipxReplicationContext(SipxReplicationContext replicationContext) {
        m_replicationContext = replicationContext;
    }
}
