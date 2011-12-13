/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.acd.stats;

import java.util.ArrayList;
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
import org.springframework.beans.factory.annotation.Required;

public class AcdStats implements FeatureProvider, AddressProvider {
    public static final LocationFeature FEATURE = new LocationFeature("acdStats");
    public static final AddressType API_ADDRESS = new AddressType("acdStatsApi");
    private BeanWithSettingsDao<AcdStatsSettings> m_settingsDao;

    public AcdStatsSettings getSettings() {
        return m_settingsDao.findOne();
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
        return Collections.singleton(API_ADDRESS);
    }

    @Override
    public Collection<Address> getAvailableAddresses(AddressManager manager, AddressType type,
            Object requester) {
        if (!type.equals(API_ADDRESS)) {
            return null;
        }

        List<Address> addresses = null;
        List<Location> locations = manager.getFeatureManager().getLocationsForEnabledFeature(FEATURE);
        if (locations != null && !locations.isEmpty()) {
            AcdStatsSettings settings = getSettings();
            addresses = new ArrayList<Address>(locations.size());
            for (Location location : locations) {
                addresses.add(new Address(location.getAddress(), settings.getAcdStatsPort()));
            }
        }
        return addresses;
    }

    @Required
    public void setSettingsDao(BeanWithSettingsDao settingsDao) {
        m_settingsDao = settingsDao;
    }
}
