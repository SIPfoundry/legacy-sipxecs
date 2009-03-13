/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.paging;

import java.util.Arrays;

import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessContext;
import org.sipfoundry.sipxconfig.admin.commserver.SipxReplicationContext;
import org.sipfoundry.sipxconfig.service.ServiceConfigurator;
import org.sipfoundry.sipxconfig.service.SipxPageService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;
import org.springframework.beans.factory.annotation.Required;

public class PagingProvisioningContextImpl implements PagingProvisioningContext {

    private SipxReplicationContext m_replicationContext;

    private SipxServiceManager m_sipxServiceManager;

    private ServiceConfigurator m_serviceConfigurator;

    private SipxProcessContext m_processContext;

    private SipxPageService m_sipxPageService;

    private void replicatePagingConfig() {
        SipxPageService pageService = (SipxPageService) m_sipxServiceManager
                .getServiceByBeanId(SipxPageService.BEAN_ID);
        m_serviceConfigurator.replicateServiceConfig(pageService);
        m_replicationContext.publishEvent(new PagingServerActivatedEvent(pageService));
    }

    /**
     * Write new configuration and restart paging server
     */
    public void deploy() {
        replicatePagingConfig();
        //mark services for restart - a reminder will be shown to the user
        m_processContext.markServicesForRestart(Arrays.asList(m_sipxPageService));
    }

    @Required
    public void setReplicationContext(SipxReplicationContext replicationContext) {
        m_replicationContext = replicationContext;
    }

    @Required
    public void setProcessContext(SipxProcessContext processContext) {
        m_processContext = processContext;
    }

    @Required
    public void setSipxServiceManager(SipxServiceManager sipxServiceManager) {
        m_sipxServiceManager = sipxServiceManager;
    }

    @Required
    public void setServiceConfigurator(ServiceConfigurator serviceConfigurator) {
        m_serviceConfigurator = serviceConfigurator;
    }

    @Required
    public void setSipxPageService(SipxPageService sipxPageService) {
        m_sipxPageService = sipxPageService;
    }
}
