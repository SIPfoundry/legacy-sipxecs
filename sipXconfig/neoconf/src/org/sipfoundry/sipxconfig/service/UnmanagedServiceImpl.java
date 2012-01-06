/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.service;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.List;

import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.address.AddressProvider;
import org.sipfoundry.sipxconfig.address.AddressType;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.feature.FeatureProvider;
import org.sipfoundry.sipxconfig.feature.GlobalFeature;
import org.sipfoundry.sipxconfig.feature.LocationFeature;
import org.sipfoundry.sipxconfig.setting.BeanWithSettingsDao;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingSet;

public class UnmanagedServiceImpl  implements FeatureProvider, AddressProvider, UnmanagedService {
    private static final List<AddressType> ADDRESSES = Arrays.asList(NTP, SYSLOG);
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
        if (!ADDRESSES.contains(type) || !manager.getFeatureManager().isFeatureEnabled(FEATURE)) {
            return null;
        }

        UnmanagedServiceSettings settings = getSettings();
        if (type.equals(NTP)) {
            SettingSet ntp = (SettingSet) settings.getSettings().getSetting("services/ntp");
            Collection<Setting> ntpServers = ntp.getValues();
            Collection<Address> addresses = new ArrayList<Address>();
            for (Setting server : ntpServers) {
                String value = server.getValue();
                if (value != null) {
                    addresses.add(new Address(value));
                }
            }
            return addresses;
        } else if (type.equals(SYSLOG)) {
            String value = settings.getSettingValue("servers/syslog");
            if (value != null) {
                return Collections.singleton(new Address(value));
            }
        }
        // TODO Auto-generated method stub
        return null;
    }

    @Override
    public Collection<GlobalFeature> getAvailableGlobalFeatures() {
        return Collections.singleton(FEATURE);
    }

    @Override
    public Collection<LocationFeature> getAvailableLocationFeatures(Location l) {
        return null;
    }

    public void setSettingsDao(BeanWithSettingsDao<UnmanagedServiceSettings> settingsDao) {
        m_settingsDao = settingsDao;
    }

}
