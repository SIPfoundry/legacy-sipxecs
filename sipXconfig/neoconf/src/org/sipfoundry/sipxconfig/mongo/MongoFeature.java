/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.mongo;

import java.util.ArrayList;
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

public class MongoFeature implements AddressProvider, FeatureProvider {
    public static final String BEAN_ID = "mongo";
    public static final AddressType ADDRESS_ID = new AddressType(BEAN_ID);
    public static final LocationFeature FEATURE_ID = new LocationFeature(BEAN_ID);
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
        Collection<Location> locations = manager.getFeatureManager().getLocationsForEnabledFeature(FEATURE_ID);
        Collection<Address> addresses = new ArrayList<Address>(locations.size());
        for (Location location : locations) {
            addresses.add(new Address(location.getAddress(), 27017));
        }
        return addresses;
    }

    @Override
    public Collection<GlobalFeature> getAvailableGlobalFeatures() {
        return null;
    }

    @Override
    public Collection<LocationFeature> getAvailableLocationFeatures(Location l) {
        return Collections.singleton(FEATURE_ID);
    }

    public void setSettingsDao(BeanWithSettingsDao<MongoSettings> settingsDao) {
        m_settingsDao = settingsDao;
    }
}
