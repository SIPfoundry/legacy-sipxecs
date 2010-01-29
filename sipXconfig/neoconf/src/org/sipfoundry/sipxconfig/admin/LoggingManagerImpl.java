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
import java.util.List;
import java.util.Map;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.acd.AcdContext;
import org.sipfoundry.sipxconfig.acd.AcdServer;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.SbcDeviceManager;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.bridge.BridgeSbc;
import org.sipfoundry.sipxconfig.service.LoggingEntity;
import org.sipfoundry.sipxconfig.service.SipxAcdService;
import org.sipfoundry.sipxconfig.service.SipxBridgeService;
import org.sipfoundry.sipxconfig.service.SipxService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.beans.factory.ListableBeanFactory;

public class LoggingManagerImpl implements LoggingManager, BeanFactoryAware {

    private static final Log LOG = LogFactory.getLog(LoggingManagerImpl.class);

    private ListableBeanFactory m_beanFactory;
    private SipxServiceManager m_sipxServiceManager;
    private LocationsManager m_locationsManager;
    private SbcDeviceManager m_sbcDeviceManager;
    private AcdContext m_acdContext;
    private List<LoggingEntity> m_entitiesToProcess;

    public Collection<LoggingEntity> getLoggingEntities() {
        Map entitiesMap = m_beanFactory.getBeansOfType(LoggingEntity.class);

        Collection<LoggingEntity> logEnabledServices = new ArrayList<LoggingEntity>();
        for (Object bean : entitiesMap.values()) {
            LoggingEntity loggingEntity = null;
            if (bean instanceof SipxService) {
                SipxService service = (SipxService) bean;
                SipxService persistedService = m_sipxServiceManager.getServiceByBeanId(service.getBeanId());
                loggingEntity = (LoggingEntity) persistedService;
            } else if (bean instanceof BridgeSbc) {
                LOG.warn("Special case of sipxbridge (BridgeSbc)");
                BridgeSbc bridgeSbc = m_sbcDeviceManager.getBridgeSbc(m_locationsManager.getPrimaryLocation());
                loggingEntity = bridgeSbc;

            } else if (bean instanceof AcdServer) {
                LOG.warn("Special case of ACD (AcdServer)");
                List servers = m_acdContext.getServers();
                if (servers.size() > 0) {
                    loggingEntity = (AcdServer) servers.get(0);
                }
            } else {
                LOG.warn("For now, only logging entities of the following types are supported: "
                        + "SipxService, BridgeSbc and AcdService ");
            }

            if (loggingEntity != null) {
                logEnabledServices.add(loggingEntity);
            }
        }
        return logEnabledServices;
    }

    public void setBeanFactory(BeanFactory beanFactory) {
        m_beanFactory = (ListableBeanFactory) beanFactory;
    }

    public void setSipxServiceManager(SipxServiceManager sipxServiceManager) {
        m_sipxServiceManager = sipxServiceManager;
    }

    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }

    public void setSbcDeviceManager(SbcDeviceManager sbcDeviceManager) {
        m_sbcDeviceManager = sbcDeviceManager;
    }

    public void setAcdContext(AcdContext acdContext) {
        m_acdContext = acdContext;
    }

    public List<LoggingEntity> getEntitiesToProcess() {
        return m_entitiesToProcess;
    }

    public void setEntitiesToProcess(List<LoggingEntity> entitiesToProcess) {
        m_entitiesToProcess = entitiesToProcess;
    }

    public SipxService getSipxServiceForLoggingEntity(LoggingEntity loggingEntity) {
        if (loggingEntity != null) {
            if (loggingEntity instanceof SipxService) {
                return (SipxService) loggingEntity;
            } else if (loggingEntity instanceof BridgeSbc) {
                return m_sipxServiceManager.getServiceByBeanId(SipxBridgeService.BEAN_ID);
            } else if (loggingEntity instanceof AcdServer) {
                return m_sipxServiceManager.getServiceByBeanId(SipxAcdService.BEAN_ID);
            }
        }
        return null;
    }
}
