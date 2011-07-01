/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.tunnel;

import java.util.Collection;

import org.sipfoundry.sipxconfig.admin.commserver.Location;

/**
 * Implementations can submit what tunnels they want to allow incoming connections to and/or
 * what tunnels they need to establish with remote machines.  Implementions simply need to be
 * a registered spring bean and they will be registered with TunnelManager.
 */
public interface TunnelProvider {

    public Collection<RemoteOutgoingTunnel> getClientSideTunnels(Collection<Location> otherLocations,
            Location thisLocation);

    public Collection<AllowedIncomingTunnel> getServerSideTunnels(Collection<Location> otherLocations,
            Location thisLocation);
}
