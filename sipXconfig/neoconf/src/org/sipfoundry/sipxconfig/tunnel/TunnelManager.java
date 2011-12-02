/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.tunnel;

import java.util.Collection;

import org.sipfoundry.sipxconfig.feature.GlobalFeature;

public interface TunnelManager {
    public static final GlobalFeature FEATURE = new GlobalFeature("tunnels");

    public Collection<TunnelProvider> getTunnelProviders();
}
