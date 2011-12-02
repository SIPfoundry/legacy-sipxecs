/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.mwi;


import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;

import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressProvider;
import org.sipfoundry.sipxconfig.address.AddressRequester;
import org.sipfoundry.sipxconfig.address.AddressType;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.feature.FeatureProvider;
import org.sipfoundry.sipxconfig.feature.GlobalFeature;
import org.sipfoundry.sipxconfig.feature.LocationFeature;
import org.sipfoundry.sipxconfig.setting.BeanWithSettingsDao;

public class Mwi implements AddressProvider, FeatureProvider {
    public static final LocationFeature FEATURE = new LocationFeature("mwi");
    public static final AddressType SIP_UDP = new AddressType("mwiSipUdp");
    public static final AddressType SIP_TCP = new AddressType("mwiSipTcp");
    public static final AddressType HTTP_API = new AddressType("mwiHttpApi");
    private static final Collection<AddressType> ADDRESSES = Arrays.asList(SIP_UDP, SIP_TCP, HTTP_API);
    private BeanWithSettingsDao<MwiSettings> m_settingsDao;

    public MwiSettings getSettings() {
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
    public Collection<AddressType> getSupportedAddressTypes() {
        return ADDRESSES;
    }

    @Override
    public Collection<Address> getAvailableAddresses(AddressType type, AddressRequester requester) {
        if (ADDRESSES.contains(type)) {
            MwiSettings settings = getSettings();
            manager.getFeatureManager().g
            if (type.equals(obj) {
                
            }
        }
    }

}
