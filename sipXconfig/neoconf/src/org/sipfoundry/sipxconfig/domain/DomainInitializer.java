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
import java.util.Date;
import java.util.Random;

import org.apache.commons.codec.binary.Base64;
import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.common.InitTaskListener;

/**
 * When system first starts up, create initial domain object w/default value(s)
 */
public class DomainInitializer extends InitTaskListener {
    private static final int SECRET_SIZE = 18;
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
        if ("initialize-domain".equals(task)) {
            initDomain();
        } else if ("initialize-domain-secret".equals(task)) {
            initDomainSecret();
        }
    }
    
    private void initDomain() {
        Domain domain = getDomain();
        m_domainManager.saveDomain(domain);
    }
    
    private void initDomainSecret() {
        Domain domain = getDomain();

        if (StringUtils.isEmpty(domain.getSharedSecret())) {
            Random random = new Random(new Date().getTime());
            byte[] secretBytes = new byte[SECRET_SIZE];
            random.nextBytes(secretBytes);
            String base64Secret = new String(new Base64().encode(secretBytes));
            domain.setSharedSecret(base64Secret);
        }
        
        m_domainManager.saveDomain(domain);
    }
    
    private Domain getDomain() {
        Domain domain = null;
        if (!m_domainManager.isDomainInitialized()) {
            domain = new Domain();
            domain.setName(getInitialDomainName());
        } else {   
            domain = m_domainManager.getDomain();
        }
        return domain;
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
