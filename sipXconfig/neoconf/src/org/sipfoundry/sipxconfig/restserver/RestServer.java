/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.restserver;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.List;

import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.address.AddressProvider;
import org.sipfoundry.sipxconfig.address.AddressRequester;
import org.sipfoundry.sipxconfig.address.AddressType;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.feature.FeatureProvider;
import org.sipfoundry.sipxconfig.feature.GlobalFeature;
import org.sipfoundry.sipxconfig.feature.LocationFeature;
import org.sipfoundry.sipxconfig.setting.BeanWithSettingsDao;

public class RestServer implements FeatureProvider, AddressProvider {
    public static final LocationFeature FEATURE = new LocationFeature("resetServer");
    public static final AddressType HTTPS_API = new AddressType("resetServerApi");
    public static final AddressType EXTERNAL_API = new AddressType("resetServerExternalApi");
    public static final AddressType SIP_TCP = new AddressType("resetServerSip");
    private static final Collection<AddressType> ADDRESSES = Arrays.asList(HTTPS_API, EXTERNAL_API, SIP_TCP);
    private BeanWithSettingsDao<RestServerSettings> m_settingsDao;

    public RestServerSettings getSettings() {
        return m_settingsDao.findOne();
    }

    @Override
    public Collection<GlobalFeature> getAvailableGlobalFeatures() {
        // TODO Auto-generated method stub
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
    public Collection<Address> getAvailableAddresses(AddressManager manager, AddressType type,
            AddressRequester requester) {
        List<Address> addresses = null;
        if (ADDRESSES.contains(type)) {
            RestServerSettings settings = getSettings();
            Collection<Location> locations = manager.getFeatureManager().getLocationsForEnabledFeature(FEATURE);
            addresses = new ArrayList<Address>(locations.size());
            for (Location location : locations) {
                Address address = new Address();
                address.setAddress(location.getAddress());
                if (type.equals(HTTPS_API)) {
                    address.setPort(settings.getHttpsPort());
                } else if (type.equals(EXTERNAL_API)) {
                    address.setPort(settings.getExternalPort());
                } else if (type.equals(SIP_TCP)) {
                    address.setPort(settings.getSipPort());
                }
                addresses.add(address);
            }
        }

        return addresses;
    }
}
