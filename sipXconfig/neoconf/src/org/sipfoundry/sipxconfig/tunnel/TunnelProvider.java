/**
 *
 *
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.tunnel;

import java.util.Collection;

import org.sipfoundry.sipxconfig.commserver.Location;

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
