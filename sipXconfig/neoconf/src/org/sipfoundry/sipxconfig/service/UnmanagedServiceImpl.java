/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.service;

import java.util.Arrays;
import java.util.Collection;
import java.util.List;

import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.address.AddressProvider;
import org.sipfoundry.sipxconfig.address.AddressType;
import org.sipfoundry.sipxconfig.setting.BeanWithSettingsDao;

public class UnmanagedServiceImpl  implements AddressProvider, UnmanagedService {
    private static final List<AddressType> ADDRESSES = Arrays.asList(NTP, SYSLOG, DNS);
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
    public Collection<AddressType> getSupportedAddressTypes(AddressManager manager) {
        return ADDRESSES;
    }

    @Override
    public Collection<Address> getAvailableAddresses(AddressManager manager, AddressType type, Object requester) {
        if (!ADDRESSES.contains(type)) {
            return null;
        }

        UnmanagedServiceSettings settings = getSettings();
        if (type.equals(NTP)) {
            return settings.getAddresses("services/ntp");
        } else if (type.equals(SYSLOG)) {
            return settings.getAddresses("servers/syslog");
        } else if (type.equals(DNS)) {
            return settings.getAddresses("services/dns");
        }

        return null;
    }

    public void setSettingsDao(BeanWithSettingsDao<UnmanagedServiceSettings> settingsDao) {
        m_settingsDao = settingsDao;
    }
}
