/*
 *
 *
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.tunnel;

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

import org.sipfoundry.sipxconfig.admin.commserver.Location;

public class TunnelClientConfigurationFile extends AbstractTunnelConfiguration {

    /**
     * Collect all the client-side tunnel configs from all the providers
     */
    protected Collection<? extends AbstractTunnel> getTunnels(Location location) {
        List<RemoteOutgoingTunnel> tunnels = new ArrayList<RemoteOutgoingTunnel>();
        List<Location> otherLocations = getOtherLocations(location);
        for (TunnelProvider p : getTunnelManager().getTunnelProviders()) {
            tunnels.addAll(p.getClientSideTunnels(otherLocations, location));
        }
        return tunnels;
    }
}
