/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.rls;

import java.util.Arrays;
import java.util.Collection;

import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressProvider;
import org.sipfoundry.sipxconfig.address.AddressRequester;
import org.sipfoundry.sipxconfig.address.AddressType;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.feature.FeatureProvider;
import org.sipfoundry.sipxconfig.feature.GlobalFeature;
import org.sipfoundry.sipxconfig.feature.LocationFeature;
import org.sipfoundry.sipxconfig.setting.BeanWithSettingsDao;

import sun.nio.ch.SocketOpts.IP.TCP;

public class Rls implements AddressProvider, FeatureProvider {
    public static final LocationFeature FEATURE = new LocationFeature("rls");
    public static final AddressType UDP_SIP = new AddressType("rlsUdp");
    public static final AddressType TCP_SIP = new AddressType("rlsTcp");
    public static final AddressType TLS_SIP = new AddressType("rlsTls");
    private static final Collection<AddressType> ADDRESSES = Arrays.asList(UDP_SIP, TCP_SIP, TLS_SIP);
    private BeanWithSettingsDao<RlsSettings> m_settingsDao;

    public RlsSettings getSettings() {
        return m_settingsDao.findOne();
    }

    @Override
    public Collection<GlobalFeature> getAvailableGlobalFeatures() {
        // TODO Auto-generated method stub
        return null;
    }

    @Override
    public Collection<LocationFeature> getAvailableLocationFeatures(Location l) {
        // TODO Auto-generated method stub
        return null;
    }

    @Override
    public Collection<AddressType> getSupportedAddressTypes() {
        return ADDRESSES;
    }

    @Override
    public Collection<Address> getAvailableAddresses(AddressType type, AddressRequester requester) {
        // TODO Auto-generated method stub
        return null;
    }
}
