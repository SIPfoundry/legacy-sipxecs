/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.openfire;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.List;

import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.address.AddressProvider;
import org.sipfoundry.sipxconfig.address.AddressType;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.feature.FeatureProvider;
import org.sipfoundry.sipxconfig.feature.GlobalFeature;
import org.sipfoundry.sipxconfig.feature.LocationFeature;
import org.sipfoundry.sipxconfig.im.ImManager;
import org.sipfoundry.sipxconfig.setting.BeanWithSettingsDao;

public class Openfire extends ImManager implements FeatureProvider, AddressProvider {
    private static final Collection<AddressType> ADDRESSES = Arrays.asList(new AddressType[] {
        XMPP_ADDRESS, XMLRPC_ADDRESS, WATCHER_ADDRESS
    });
    private BeanWithSettingsDao<OpenfireSettings> m_settingsDao;
    
    public OpenfireSettings getSettings() {
        return m_settingsDao.findOrCreateOne();
    }
    
    public void updateSettings(OpenfireSettings settings) {
        m_settingsDao.upsert(settings);
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
    public Collection<AddressType> getSupportedAddressTypes(AddressManager manager) {
        return ADDRESSES;
    }

    @Override
    public Collection<Address> getAvailableAddresses(AddressManager manager, AddressType type,
            Object requester) {
        if (!ADDRESSES.contains(type)) {
            return null;
        }
        Collection<Location> locations = manager.getFeatureManager().getLocationsForEnabledFeature(FEATURE);
        // only zero or one at this time allowed
        if (locations.isEmpty()) {
            return null;
        }

        OpenfireSettings settings = getSettings();
        List<Address> addresses = new ArrayList<Address>(locations.size());
        for (Location location : locations) {
            Address address = new Address();
            address.setAddress(location.getAddress());
            if (type.equals(XMPP_ADDRESS)) {
                address.setPort(settings.getXmppPort());
            } else if (type.equals(XMLRPC_ADDRESS)) {
                address.setPort(settings.getXmlRpcPort());
                address.setFormat("http://%s:%d/xmlrpc");
            } else if (type.equals(WATCHER_ADDRESS)) {
                address.setPort(settings.getWatcherPort());
            }
            addresses.add(address);
        }
        
        return addresses;            
    }
}
