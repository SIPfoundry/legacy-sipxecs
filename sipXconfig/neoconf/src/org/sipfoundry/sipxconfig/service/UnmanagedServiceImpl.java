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
package org.sipfoundry.sipxconfig.service;


import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.List;

import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.address.AddressProvider;
import org.sipfoundry.sipxconfig.address.AddressType;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.firewall.DefaultFirewallRule;
import org.sipfoundry.sipxconfig.firewall.FirewallManager;
import org.sipfoundry.sipxconfig.firewall.FirewallProvider;
import org.sipfoundry.sipxconfig.firewall.FirewallRule;
import org.sipfoundry.sipxconfig.setting.BeanWithSettingsDao;

public class UnmanagedServiceImpl  implements AddressProvider, UnmanagedService, FirewallProvider {
    private static final List<AddressType> ADDRESSES = Arrays.asList(SYSLOG, DNS, SSH);
    private BeanWithSettingsDao<UnmanagedServiceSettings> m_settingsDao;

    @Override
    public UnmanagedServiceSettings getSettings() {
        return m_settingsDao.findOrCreateOne();
    }

    @Override
    public void saveSettings(UnmanagedServiceSettings settings) {
        m_settingsDao.upsert(settings);
    }

    @Override
    public Collection<Address> getAvailableAddresses(AddressManager manager, AddressType type, Location requester) {
        if (!ADDRESSES.contains(type)) {
            return null;
        }

        UnmanagedServiceSettings settings = getSettings();
        if (type.equals(SYSLOG)) {
            return settings.getAddresses(SYSLOG, "services/syslog");
        } else if (type.equals(DNS)) {
            return settings.getAddresses(DNS, "services/dns");
        } else if (type.equals(SSH)) {
            // return the ssh server on the server that asking. mostly useful for firewall rules
            return Collections.singleton(new Address(SSH, requester.getAddress()));
        }

        return null;
    }

    public void setSettingsDao(BeanWithSettingsDao<UnmanagedServiceSettings> settingsDao) {
        m_settingsDao = settingsDao;
    }

    @Override
    public Collection<DefaultFirewallRule> getFirewallRules(FirewallManager manager) {
        return Collections.singleton(new DefaultFirewallRule(SSH, FirewallRule.SystemId.PUBLIC));
    }
}
