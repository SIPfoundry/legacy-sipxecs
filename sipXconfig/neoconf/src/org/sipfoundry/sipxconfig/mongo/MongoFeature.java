/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.mongo;

import java.util.Collection;
import java.util.Collections;

import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressProvider;
import org.sipfoundry.sipxconfig.address.AddressType;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.feature.FeatureProvider;
import org.sipfoundry.sipxconfig.feature.GlobalFeature;
import org.sipfoundry.sipxconfig.feature.LocationFeature;

public class MongoFeature implements AddressProvider, FeatureProvider {
    public static final String BEAN_ID = "mongo";
    public static final AddressType ADDRESS_ID = new AddressType(BEAN_ID);
    public static final LocationFeature FEATURE_ID = new LocationFeature(BEAN_ID);
    private FeatureManager m_featureManager;

    @Override
    public Collection<AddressType> getSupportedAddressTypes() {
        return Collections.singleton(ADDRESS_ID);
    }

    @Override
    public Collection<Address> getAvailableAddresses(Location location, AddressType type) {
        Collection<Address> addresses = null;
        if (m_featureManager.isLocationFeatureEnabled(FEATURE_ID, location)) {
            addresses = Collections.singleton(new Address(location.getAddress(), 27017));
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
}
