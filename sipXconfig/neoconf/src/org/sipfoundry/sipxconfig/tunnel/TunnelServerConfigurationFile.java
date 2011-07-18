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

public class TunnelServerConfigurationFile extends AbstractTunnelConfiguration {

    /**
     * Collect all the server-side tunnel configs from all the providers
     */
    protected Collection<? extends AbstractTunnel> getTunnels(Location location) {
        List<AllowedIncomingTunnel> tunnels = new ArrayList<AllowedIncomingTunnel>();
        List<Location> otherLocations = getOtherLocations(location);
        for (TunnelProvider p : getTunnelManager().getTunnelProviders()) {
            tunnels.addAll(p.getServerSideTunnels(otherLocations, location));
        }
        return tunnels;
    }
}
