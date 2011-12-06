/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.registrar;

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
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.beans.factory.ListableBeanFactory;

public class Registrar implements FeatureProvider, AddressProvider, BeanFactoryAware {
    public static final LocationFeature FEATURE = new LocationFeature("registrar");
    public static final AddressType TCP_ADDRESS = new AddressType("registrar-tcp");
    public static final AddressType EVENT_ADDRESS = new AddressType("registrar-event");
    public static final AddressType UDP_ADDRESS = new AddressType("registrar-udp");
    public static final AddressType PRESENCE_MONITOR_ADDRESS = new AddressType("registrar-presence");

    private static final Collection<AddressType> ADDRESSES = Arrays.asList(new AddressType[] {
        TCP_ADDRESS, UDP_ADDRESS, PRESENCE_MONITOR_ADDRESS, EVENT_ADDRESS
    });
    private BeanWithSettingsDao<RegistrarSettings> m_settingsDao;
    private ListableBeanFactory m_beanFactory;

    @Override
    public Collection<GlobalFeature> getAvailableGlobalFeatures() {
        return null;
    }

    @Override
    public Collection<LocationFeature> getAvailableLocationFeatures(Location l) {
        return Collections.singleton(FEATURE);
    }

    public RegistrarSettings getSettings() {
        return m_settingsDao.findOne();
    }

    public void initialize() {
        RegistrarSettings settings = getSettings();
        if (settings != null) {
            return;
        }

        settings = m_beanFactory.getBean(RegistrarSettings.class);
        m_settingsDao.upsert(settings);
    }

    @Override
    public Collection<AddressType> getSupportedAddressTypes(AddressManager manager) {
        return ADDRESSES;
    }

    @Override
    public Collection<Address> getAvailableAddresses(AddressManager manager, AddressType type,
            AddressRequester requester) {
        if (ADDRESSES.contains(type)) {
            RegistrarSettings settings = getSettings();
            Collection<Location> locations = manager.getFeatureManager().getLocationsForEnabledFeature(FEATURE);
            List<Address> addresses = new ArrayList<Address>(locations.size());
            for (Location location : locations) {
                Address address = new Address();
                address.setAddress(location.getAddress());
                if (type.equals(TCP_ADDRESS)) {
                    address.setPort(settings.get);
                } else if (type.equals(UDP_ADDRESS)) {
                    
                } else if (type.equals(EVENT_ADDRESS)) {
                    
                } else if (type.equals(PRESENCE_MONITOR_ADDRESS)) {
                    
                }
            }
        }

        
        String fqdn = location.getFqdn();
        if (multipleRegistrars) {
            // more than one service - use DNS SRV
            return "rr." + fqdn;
        }
        return String.format("%s:%s;transport=tcp", fqdn, getSipPort());

        return null;
    }

    @Override
    public void setBeanFactory(BeanFactory beanFactory) {
        m_beanFactory = (ListableBeanFactory) beanFactory;
    }
}
