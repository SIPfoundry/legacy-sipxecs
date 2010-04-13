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

import org.sipfoundry.sipxconfig.admin.commserver.LocationsMigrationTrigger;
import org.sipfoundry.sipxconfig.admin.commserver.imdb.ReplicationManagerImpl;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.springframework.context.ApplicationEvent;
import org.springframework.context.ApplicationListener;

/**
 * Hook to mimic what a real system does when it starts up, and that is to create initial,
 * required, system data.
 */
public class InitializeTestSystem implements ApplicationListener {
    private ReplicationManagerImpl m_replicationManagerImpl;
    private DomainManager m_domainManager;
    private LocationsMigrationTrigger m_locationsMigrationTrigger;

    public void onApplicationEvent(ApplicationEvent event) {
        System.setProperty("sipxconfig.hostname", "sipx.example.org");
        m_replicationManagerImpl.setEnabled(false);
        m_locationsMigrationTrigger.onInitTask("migrate_locations");
        m_domainManager.initializeDomain();

    }

    public void setReplicationManagerImpl(ReplicationManagerImpl replicationManagerImpl) {
        m_replicationManagerImpl = replicationManagerImpl;
    }

    public void setDomainManager(DomainManager domainManager) {
        m_domainManager = domainManager;
    }

    public void setLocationsMigrationTrigger(LocationsMigrationTrigger locationsMigrationTrigger) {
        m_locationsMigrationTrigger = locationsMigrationTrigger;
    }
}
