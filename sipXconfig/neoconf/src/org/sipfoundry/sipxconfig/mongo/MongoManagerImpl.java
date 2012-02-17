/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.mongo;


import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;

import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.address.AddressProvider;
import org.sipfoundry.sipxconfig.address.AddressType;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.feature.FeatureProvider;
import org.sipfoundry.sipxconfig.feature.GlobalFeature;
import org.sipfoundry.sipxconfig.feature.LocationFeature;
import org.sipfoundry.sipxconfig.setting.BeanWithSettingsDao;

public class MongoManagerImpl implements AddressProvider, FeatureProvider, MongoManager {
    private BeanWithSettingsDao<MongoSettings> m_settingsDao;

    public MongoSettings getSettings() {
        return m_settingsDao.findOrCreateOne();
    }

    public void saveSettings(MongoSettings settings) {
        m_settingsDao.upsert(settings);
    }

    @Override
    public Collection<AddressType> getSupportedAddressTypes(AddressManager manager) {
        return Collections.singleton(ADDRESS_ID);
    }

    @Override
    public Collection<Address> getAvailableAddresses(AddressManager manager, AddressType type,
            Object requester) {
        if (!type.equalsAnyOf(ADDRESS_ID)) {
            return null;
        }
        Collection<Location> locations = manager.getFeatureManager().getLocationsForEnabledFeature(FEATURE_ID);
        Collection<Address> addresses = new ArrayList<Address>(locations.size());
        for (Location location : locations) {
            addresses.add(new Address(ADDRESS_ID, location.getAddress(), 27017));
        }
        return addresses;
    }

    @Override
    public Collection<GlobalFeature> getAvailableGlobalFeatures() {
        return null;
    }

    @Override
    public Collection<LocationFeature> getAvailableLocationFeatures(Location l) {
        if (l.isPrimary()) {
            return Collections.singleton(ARBITER_FEATURE);
        }
        return Arrays.asList(FEATURE_ID, ARBITER_FEATURE);
    }

    public void setSettingsDao(BeanWithSettingsDao<MongoSettings> settingsDao) {
        m_settingsDao = settingsDao;
    }
}
