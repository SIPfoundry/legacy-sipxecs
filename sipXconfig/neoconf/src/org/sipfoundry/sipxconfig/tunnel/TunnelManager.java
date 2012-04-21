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

import org.sipfoundry.sipxconfig.feature.GlobalFeature;

public interface TunnelManager {
    public static final GlobalFeature FEATURE = new GlobalFeature("tunnel");

    public Collection<TunnelProvider> getTunnelProviders();

    public TunnelSettings getSettings();

    public void saveSettings(TunnelSettings settings);
}
