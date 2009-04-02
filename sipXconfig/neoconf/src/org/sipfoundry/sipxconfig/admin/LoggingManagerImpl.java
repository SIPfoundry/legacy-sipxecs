/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Map;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.service.LoggingEntity;
import org.sipfoundry.sipxconfig.service.SipxService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.beans.factory.ListableBeanFactory;

public class LoggingManagerImpl implements LoggingManager, BeanFactoryAware {

    private static final Log LOG = LogFactory.getLog(LoggingManagerImpl.class);

    private ListableBeanFactory m_beanFactory;
    private SipxServiceManager m_sipxServiceManager;

    public Collection<LoggingEntity> getLoggingEntities() {
        Map entitiesMap = m_beanFactory.getBeansOfType(LoggingEntity.class);

        Collection<LoggingEntity> logEnabledServices = new ArrayList<LoggingEntity>();
        for (Object bean : entitiesMap.values()) {
            if (!(bean instanceof SipxService)) {
                LOG.warn("Only logging entities of type SipxService are supported at this time.");
                continue;
            }
            SipxService service = (SipxService) bean;
            SipxService persistedService = m_sipxServiceManager.getServiceByBeanId(service.getBeanId());
            LoggingEntity loggingEntity = (LoggingEntity) persistedService;
            logEnabledServices.add(loggingEntity);
        }

        return logEnabledServices;
    }

    public void setBeanFactory(BeanFactory beanFactory) {
        m_beanFactory = (ListableBeanFactory) beanFactory;
    }

    public void setSipxServiceManager(SipxServiceManager sipxServiceManager) {
        m_sipxServiceManager = sipxServiceManager;
    }
}
