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

import org.sipfoundry.sipxconfig.admin.commserver.SipxReplicationContext;
import org.sipfoundry.sipxconfig.service.ServiceConfigurator;
import org.sipfoundry.sipxconfig.service.SipxPageService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;
import org.springframework.beans.factory.annotation.Required;

public class PagingProvisioningContextImpl implements PagingProvisioningContext {

    private SipxReplicationContext m_replicationContext;

    private SipxServiceManager m_sipxServiceManager;

    private ServiceConfigurator m_serviceConfigurator;

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
    }

    @Required
    public void setReplicationContext(SipxReplicationContext replicationContext) {
        m_replicationContext = replicationContext;
    }

    @Required
    public void setSipxServiceManager(SipxServiceManager sipxServiceManager) {
        m_sipxServiceManager = sipxServiceManager;
    }

    @Required
    public void setServiceConfigurator(ServiceConfigurator serviceConfigurator) {
        m_serviceConfigurator = serviceConfigurator;
    }
}
