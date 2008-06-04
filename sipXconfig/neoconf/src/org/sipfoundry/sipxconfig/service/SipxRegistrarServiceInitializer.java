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

import org.sipfoundry.sipxconfig.common.InitTaskListener;
import org.sipfoundry.sipxconfig.domain.DomainManager;

public class SipxRegistrarServiceInitializer extends InitTaskListener {

    private SipxServiceManager m_sipxServiceManager;
    private DomainManager m_domainManager;

    @Override
    public void onInitTask(String task) {
        Collection<String> domainAliases = m_domainManager.getDomain().getAliases();
        SipxRegistrarService registrarService = (SipxRegistrarService) m_sipxServiceManager
                .getServiceByBeanId(SipxRegistrarService.BEAN_ID);
        registrarService.setRegistrarDomainAliases(domainAliases);
        m_sipxServiceManager.storeService(registrarService);
    }

    public void setSipxServiceManager(SipxServiceManager sipxServiceManager) {
        m_sipxServiceManager = sipxServiceManager;
    }

    public void setDomainManager(DomainManager domainManager) {
        m_domainManager = domainManager;
    }
}
