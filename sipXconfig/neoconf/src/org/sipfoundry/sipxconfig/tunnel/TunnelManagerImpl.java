/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.tunnel;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Map;

import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.admin.commserver.SipxReplicationContext;
import org.sipfoundry.sipxconfig.common.event.DaoEventListener;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.beans.factory.BeanInitializationException;
import org.springframework.beans.factory.ListableBeanFactory;

/**
 * Aggregate all tunnels for all locations and services and write them to stunnel config files
 * that map ports in and out.  This is useful for allowing services on one machine to securely communicate
 * with services on another machine without allowing unauthorized connections for services that either
 * don't have authentication mechanisms or are to cumbersome to configure such as the mongo database service.
 */
public abstract class TunnelManagerImpl implements TunnelManager, BeanFactoryAware, DaoEventListener {
    private ListableBeanFactory m_beanFactory;
    private volatile Collection<TunnelProvider> m_providers;
    private LocationsManager m_locationsManager;
    private TunnelClientConfigurationFile m_clientFile;
    private TunnelServerConfigurationFile m_serverFile;
    protected abstract SipxReplicationContext getSipxReplicationContext();

    @Override
    public void setBeanFactory(BeanFactory beanFactory) {
        m_beanFactory = (ListableBeanFactory) beanFactory;
    }

    public LocationsManager getLocationsManager() {
        return m_locationsManager;
    }

    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }

    void locationRemoved(Location l) {
        getSipxReplicationContext().replicate(m_locationsManager.getLocations(), m_clientFile);
    }

    void locationChanged(Location l) {
        getSipxReplicationContext().replicate(m_locationsManager.getLocations(), m_clientFile);
        getSipxReplicationContext().replicate(l, m_serverFile);
    }

    public void onDelete(Object entity) {
        if (entity instanceof Location) {
            locationRemoved((Location) entity);
        }
    }

    public void onSave(Object entity) {
        if (entity instanceof Location) {
            Location l = (Location) entity;
            if (l.hasFqdnOrIpChangedOnSave()) {
                locationChanged(l);
            }
        }
    }

    public TunnelClientConfigurationFile getClientFile() {
        return m_clientFile;
    }

    public void setClientFile(TunnelClientConfigurationFile clientFile) {
        m_clientFile = clientFile;
    }

    public TunnelServerConfigurationFile getServerFile() {
        return m_serverFile;
    }

    public void setServerFile(TunnelServerConfigurationFile serverFile) {
        m_serverFile = serverFile;
    }

    /**
     * List of registered providers. Lazily get them from spring.
     */
    @Override
    public Collection<TunnelProvider> getTunnelProviders() {
        if (m_providers == null) {
            if (m_beanFactory == null) {
                throw new BeanInitializationException(getClass().getName() + " not initialized");
            }
            Map<String, TunnelProvider> beanMap = m_beanFactory.getBeansOfType(TunnelProvider.class, false,
                    true);
            m_providers = new ArrayList<TunnelProvider>(beanMap.values());
        }

        return m_providers;
    }

    public void setProviders(Collection<TunnelProvider> providers) {
        m_providers = providers;
    }

}
