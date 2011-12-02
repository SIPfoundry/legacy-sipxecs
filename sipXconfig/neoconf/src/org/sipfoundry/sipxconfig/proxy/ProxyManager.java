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
import org.sipfoundry.sipxconfig.address.AddressProvider;
import org.sipfoundry.sipxconfig.address.AddressType;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.feature.FeatureProvider;
import org.sipfoundry.sipxconfig.feature.GlobalFeature;
import org.sipfoundry.sipxconfig.feature.LocationFeature;
import org.sipfoundry.sipxconfig.setting.BeanWithSettingsDao;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.beans.factory.ListableBeanFactory;

public class ProxyManager implements FeatureProvider, AddressProvider, BeanFactoryAware {
    public static final String ID = "proxy";
    public static final LocationFeature FEATURE = new LocationFeature(ID);
    public static final AddressType TCP_ADDRESS = new AddressType("proxyTcp");
    public static final AddressType UDP_ADDRESS = new AddressType("procyUdp");
    public static final AddressType TLS_ADDRESS = new AddressType("proxyTls");
    private static final Collection<AddressType> ADDRESS_TYPES = Arrays.asList(new AddressType[] {
        TCP_ADDRESS, UDP_ADDRESS, TLS_ADDRESS
    });
    private FeatureManager m_featureManager;
    private BeanWithSettingsDao<ProxySettings> m_settingsDoa;
    private ListableBeanFactory m_beanFactory;

    public void initialize() {
        ProxySettings settings = getSettings();
        if (settings == null) {
            settings = m_beanFactory.getBean(ProxySettings.class);
            m_settingsDoa.upsert(settings);
        }
    }

    @Override
    public Collection<GlobalFeature> getAvailableGlobalFeatures() {
        return null;
    }

    @Override
    public Collection<LocationFeature> getAvailableLocationFeatures(Location l) {
        return Collections.singleton(FEATURE);
    }

    public ProxySettings getSettings() {
        return m_settingsDoa.findOne();
    }

    @Override
    public Collection<AddressType> getSupportedAddressTypes() {
        return ADDRESS_TYPES;
    }

    @Override
    public Collection<Address> getAvailableAddresses(AddressType type) {
        Collection<Address> addresses = null;
        if (ADDRESS_TYPES.contains(type)) {
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
            }
        }

        return addresses;
    }

    @Override
    public void setBeanFactory(BeanFactory beanFactory) {
        m_beanFactory = (ListableBeanFactory) beanFactory;
    }
}
