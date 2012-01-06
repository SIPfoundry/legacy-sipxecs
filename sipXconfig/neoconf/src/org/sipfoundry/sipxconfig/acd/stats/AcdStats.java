/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.acd.stats;

import org.sipfoundry.sipxconfig.address.AddressType;
import org.sipfoundry.sipxconfig.feature.LocationFeature;

public interface AcdStats {
    public static final LocationFeature FEATURE = new LocationFeature("acdStats");
    public static final AddressType API_ADDRESS = new AddressType("acdStatsApi");

    public AcdStatsSettings getSettings();

    public void saveSettings(AcdStatsSettings settings);
}
