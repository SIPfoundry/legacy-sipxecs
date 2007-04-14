/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site;

import org.sipfoundry.sipxconfig.admin.commserver.imdb.ReplicationManagerImpl;
import org.sipfoundry.sipxconfig.domain.DomainInitializer;
import org.springframework.context.ApplicationEvent;
import org.springframework.context.ApplicationListener;

/**
 * Hook to mimic what a real system does when it starts up, and that is to create
 * initial, required, system data.
 */
public class InitializeTestSystem implements ApplicationListener {
    private DomainInitializer m_domainInitializer;
    private ReplicationManagerImpl m_replicationManagerImpl;

    public DomainInitializer getDomainInitializer() {
        return m_domainInitializer;
    }

    public void setDomainInitializer(DomainInitializer domainInitializer) {
        m_domainInitializer = domainInitializer;
    }

    public void onApplicationEvent(ApplicationEvent event) {
        // decided to pick a ficticious name rather than allowing default hostname to be used
        // this should simplify unit tests by letting them validate against exact values.
        m_domainInitializer.setInitialDomain("example.org");
        m_domainInitializer.onInitTask(null);
        
        m_replicationManagerImpl.setEnabled(false);
    }

    public void setReplicationManagerImpl(ReplicationManagerImpl replicationManagerImpl) {
        m_replicationManagerImpl = replicationManagerImpl;
    }
}
