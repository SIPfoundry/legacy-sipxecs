/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.im;

import java.util.Arrays;
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

public class ImManager implements FeatureProvider, AddressProvider {
    public static final String ID = "im"; // only define public to avoid check style error
    public static final LocationFeature FEATURE = new LocationFeature(ID);
    public static final AddressType XMPP_ADDRESS = new AddressType(ID);
    public static final AddressType XMLRPC_ADDRESS = new AddressType("im-xmlrpc");
    private static final Collection<AddressType> ADDRESSES = Arrays.asList(new AddressType[] {
        XMPP_ADDRESS, XMLRPC_ADDRESS
    });
    private static final int XMPP_PORT = 1234; // not configurable
    private static final int XMLRPC_PORT = 9094; // not configurable

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
        Collection<Address> addresses = null;
        if (ADDRESSES.contains(type)) {
            Collection<Location> locations = manager.getFeatureManager().getLocationsForEnabledFeature(FEATURE);
            // only zero or one at this time allowed
            if (locations.size() > 0) {
                Location location = locations.iterator().next();
                Address address = new Address();
                address.setAddress(location.getAddress());
                if (type.equals(XMPP_ADDRESS)) {
                    address.setPort(XMPP_PORT);
                } else {
                    address.setPort(XMLRPC_PORT);
                    address.setFormat("http://%s:%d/xmlrpc");
                }
                addresses = Collections.singleton(address);
            }
        }
        return addresses;
    }
}
