/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.domain;

import java.net.InetAddress;
import java.net.UnknownHostException;

import org.apache.commons.lang.RandomStringUtils;
import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.common.InitTaskListener;
import org.sipfoundry.sipxconfig.domain.DomainManager.DomainNotInitializedException;

/**
 * When system first starts up, create initial domain object w/default value(s)
 */
public class DomainInitializer extends InitTaskListener {
    private DomainManager m_domainManager;
    private String m_initialDomain;

    public void setInitialDomain(String initialDomain) {
        m_initialDomain = initialDomain;
    }

    public void setDomainManager(DomainManager domainManager) {
        m_domainManager = domainManager;
    }

    /**
     * Initialize the domain on the first time that sipXconfig is run.  If there
     * is already an existing domain instance returned by the domain manager, do
     * not re-initialize
     */
    public void onInitTask(String task) {
        Domain domain = null;
        try {
            domain = m_domainManager.getDomain();
        } catch (DomainNotInitializedException e) {
            domain = new Domain();
            domain.setName(getInitialDomainName());
        }
        
        if (StringUtils.isEmpty(domain.getSharedSecret())) {
            String sharedSecret = RandomStringUtils.randomAscii(18);
            domain.setSharedSecret(sharedSecret);
        }
        
        m_domainManager.saveDomain(domain);
    }
    
    String getInitialDomainName() {
        if (m_initialDomain != null) {
            return m_initialDomain;
        }

        try {
            InetAddress addr = InetAddress.getLocalHost();
            m_initialDomain = addr.getHostName();
        } catch (UnknownHostException e) {
            throw new RuntimeException("Could not determine hostname", e);
        }
        return m_initialDomain;
    }
}
