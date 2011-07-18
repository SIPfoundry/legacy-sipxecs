/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.tunnel;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.List;

import org.apache.velocity.VelocityContext;
import org.sipfoundry.sipxconfig.admin.TemplateConfigurationFile;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;

public abstract class AbstractTunnelConfiguration extends TemplateConfigurationFile  {
    private TunnelManager m_tunnelManager;
    private LocationsManager m_locationsManager;

    public AbstractTunnelConfiguration() {
        super();
    }

    @Override
    protected VelocityContext setupContext(Location location) {
        VelocityContext context = super.setupContext(location);
        context.put("tunnelManager", m_tunnelManager);
        context.put("tunnels", getTunnels(location));
        return context;
    }

    protected abstract Collection<? extends AbstractTunnel> getTunnels(Location location);

    protected List<Location> getOtherLocations(Location location) {
        // Although it should be only locations that have stunnel service running, if the
        // system is unregistered, service will not be running, but we'll want to generate
        // the configuration for it.  Also, providers and ultimately control what the actual
        // tunnels are anyway
        Location[] locations = m_locationsManager.getLocations();
        List<Location> otherLocations = new ArrayList<Location>(Arrays.asList(locations));
        otherLocations.remove(location);
        return otherLocations;
    }

    public TunnelManager getTunnelManager() {
        return m_tunnelManager;
    }

    public void setTunnelManager(TunnelManager tunnelManager) {
        m_tunnelManager = tunnelManager;
    }

    public LocationsManager getLocationsManager() {
        return m_locationsManager;
    }

    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }
}
