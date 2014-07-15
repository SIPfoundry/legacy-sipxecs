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
package org.sipfoundry.sipxconfig.saa;

import java.util.Arrays;
import java.util.Collection;
import java.util.List;

import org.sipfoundry.sipxconfig.address.AddressType;
import org.sipfoundry.sipxconfig.feature.LocationFeature;
import org.sipfoundry.sipxconfig.firewall.DefaultFirewallRule;
import org.sipfoundry.sipxconfig.setting.PersistableSettings;

public interface SaaManager {
    final AddressType SAA_TCP = new AddressType("saaTcp");
    final AddressType SAA_UDP = new AddressType("saaUdp");
    final List<AddressType> SUPPORTED_ADDRESS_TYPES = Arrays.asList(new AddressType[] {
        SAA_TCP, SAA_UDP
    });

    final Collection<DefaultFirewallRule> DEFAULT_RULES = DefaultFirewallRule.rules(SUPPORTED_ADDRESS_TYPES);

    final LocationFeature FEATURE = new LocationFeature("saa");

    PersistableSettings getSettings();

    void saveSettings(PersistableSettings settings);
}
