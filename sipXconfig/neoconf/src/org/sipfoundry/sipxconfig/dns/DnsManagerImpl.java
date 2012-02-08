/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.dns;

import static java.lang.String.format;

import java.util.Collection;
import java.util.Collections;
import java.util.Iterator;
import java.util.List;

import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.address.AddressProvider;
import org.sipfoundry.sipxconfig.address.AddressType;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.feature.FeatureProvider;
import org.sipfoundry.sipxconfig.feature.GlobalFeature;
import org.sipfoundry.sipxconfig.feature.LocationFeature;
import org.sipfoundry.sipxconfig.registrar.Registrar;
import org.sipfoundry.sipxconfig.setting.BeanWithSettingsDao;

public class DnsManagerImpl implements DnsManager, AddressProvider, FeatureProvider {
    private BeanWithSettingsDao<DnsSettings> m_settingsDao;

    @Override
    public Address getSingleAddress(AddressType t, Collection<Address> addresses, Location whoIsAsking) {
        if (addresses == null || addresses.size() == 0) {
            return null;
        }

        Iterator<Address> i = addresses.iterator();
        Address first = i.next();
        if (addresses.size() == 1 || whoIsAsking == null) {
            return first;
        }

        // registrar is only service that supports resource records so hardcode
        // that logic in here. make this pluggable at some point.
        if (t.equals(Registrar.TCP_ADDRESS)) {
            // NOTE: drop port, it's in DNS
            return new Address(t, format("rr.%s", whoIsAsking.getFqdn()));
        }

        // return the address local to who is asking if available
        Address a = first;
        while (a != null) {
            if (a.getAddress().equals(whoIsAsking.getAddress())) {
                return a;
            }
            a = (i.hasNext() ? i.next() : null);
        }

        // first is as good as any other
        return first;
    }

    @Override
    public Collection<AddressType> getSupportedAddressTypes(AddressManager manager) {
        return Collections.singleton(DNS_ADDRESS);
    }

    @Override
    public Collection<Address> getAvailableAddresses(AddressManager manager, AddressType type, Object requester) {
        if (!type.equals(DNS_ADDRESS)) {
            return null;
        }
        List<Location> locations = manager.getFeatureManager().getLocationsForEnabledFeature(FEATURE);
        return Location.toAddresses(DNS_ADDRESS, locations);
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
    public DnsSettings getSettings() {
        return m_settingsDao.findOrCreateOne();
    }

    @Override
    public void saveSettings(DnsSettings settings) {
        m_settingsDao.upsert(settings);
    }

    public void setSettingsDao(BeanWithSettingsDao<DnsSettings> settingsDao) {
        m_settingsDao = settingsDao;
    }
}
