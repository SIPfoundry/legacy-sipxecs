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

    public void onApplicationEvent(ApplicationEvent event) {
        m_domainInitializer.onInitTask(null);
        
        m_replicationManagerImpl.setEnabled(false);
    }

    public void setDomainInitializer(DomainInitializer domainInitializer) {
        m_domainInitializer = domainInitializer;
    }

    public void setReplicationManagerImpl(ReplicationManagerImpl replicationManagerImpl) {
        m_replicationManagerImpl = replicationManagerImpl;
    }
}
