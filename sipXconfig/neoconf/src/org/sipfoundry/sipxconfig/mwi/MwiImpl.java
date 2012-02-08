/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.mwi;

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

public class MwiImpl implements AddressProvider, FeatureProvider, Mwi {
    private static final Collection<AddressType> ADDRESSES = Arrays.asList(SIP_UDP, SIP_TCP, HTTP_API);
    private BeanWithSettingsDao<MwiSettings> m_settingsDao;

    public MwiSettings getSettings() {
        return m_settingsDao.findOrCreateOne();
    }

    public void saveSettings(MwiSettings settings) {
        m_settingsDao.upsert(settings);
    }

    @Override
    public Collection<GlobalFeature> getAvailableGlobalFeatures() {
        return null;
    }

    @Override
    public Collection<LocationFeature> getAvailableLocationFeatures(Location l) {
        return Collections.singleton(FEATURE);
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

        MwiSettings settings = getSettings();
        List<Location> locations = manager.getFeatureManager().getLocationsForEnabledFeature(FEATURE);
        List<Address> addresses = new ArrayList<Address>(locations.size());
        for (Location location : locations) {
            Address address;
            if (type.equals(HTTP_API)) {
                address = new Address(HTTP_API, location.getAddress(), settings.getHttpApiPort());
            } else if (type.equals(SIP_UDP)) {
                address = new Address(SIP_UDP, location.getAddress(), settings.getUdpPort());
            } else {
                address = new Address(SIP_TCP, location.getAddress(), settings.getTcp());
            }
            addresses.add(address);
        }
        return addresses;
    }

    public void setSettingsDao(BeanWithSettingsDao<MwiSettings> settingsDao) {
        m_settingsDao = settingsDao;
    }
}
