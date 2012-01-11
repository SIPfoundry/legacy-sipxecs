/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.proxy;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;

import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.address.AddressProvider;
import org.sipfoundry.sipxconfig.address.AddressType;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.feature.FeatureProvider;
import org.sipfoundry.sipxconfig.feature.GlobalFeature;
import org.sipfoundry.sipxconfig.feature.LocationFeature;
import org.sipfoundry.sipxconfig.setting.BeanWithSettingsDao;

public class ProxyManagerImpl implements ProxyManager, FeatureProvider, AddressProvider {
    public static final LocationFeature FEATURE = new LocationFeature("proxy");
    public static final AddressType TCP_ADDRESS = new AddressType("proxyTcp");
    public static final AddressType UDP_ADDRESS = new AddressType("procyUdp");
    public static final AddressType TLS_ADDRESS = new AddressType("proxyTls");
    private static final Collection<AddressType> ADDRESS_TYPES = Arrays.asList(new AddressType[] {
        TCP_ADDRESS, UDP_ADDRESS, TLS_ADDRESS
    });
    private FeatureManager m_featureManager;
    private BeanWithSettingsDao<ProxySettings> m_settingsDao;

    @Override
    public Collection<GlobalFeature> getAvailableGlobalFeatures() {
        return null;
    }

    @Override
    public Collection<LocationFeature> getAvailableLocationFeatures(Location l) {
        return Collections.singleton(FEATURE);
    }

    public ProxySettings getSettings() {
        return m_settingsDao.findOrCreateOne();
    }

    public void saveSettings(ProxySettings settings) {
        m_settingsDao.upsert(settings);
    }

    @Override
    public Collection<AddressType> getSupportedAddressTypes(AddressManager manager) {
        return ADDRESS_TYPES;
    }

    @Override
    public Collection<Address> getAvailableAddresses(AddressManager manager, AddressType type,
            Object requester) {
        if (!ADDRESS_TYPES.contains(type)) {
            return null;
        }
        Collection<Address> addresses = null;
        Collection<Location> locations = m_featureManager.getLocationsForEnabledFeature(FEATURE);
        addresses = new ArrayList<Address>(locations.size());
        for (Location location : locations) {
            Address address = new Address();
            address.setAddress(location.getAddress());
            if (type.equals(TCP_ADDRESS) || type.equals(UDP_ADDRESS)) {
                address.setPort(5060);
            } else if (type.equals(TLS_ADDRESS)) {
                address.setPort(5061);
            }
            address.setAddress("sip:%s:%d");
            addresses.add(address);
        }

        return addresses;
    }

    public void setFeatureManager(FeatureManager featureManager) {
        m_featureManager = featureManager;
    }

    public void setSettingsDao(BeanWithSettingsDao<ProxySettings> settingsDao) {
        m_settingsDao = settingsDao;
    }
}
