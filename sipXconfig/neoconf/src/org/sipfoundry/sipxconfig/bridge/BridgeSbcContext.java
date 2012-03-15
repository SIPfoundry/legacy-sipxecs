/**
 *
 *
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.bridge;


import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.List;

import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.address.AddressProvider;
import org.sipfoundry.sipxconfig.address.AddressType;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.feature.Bundle;
import org.sipfoundry.sipxconfig.feature.FeatureProvider;
import org.sipfoundry.sipxconfig.feature.GlobalFeature;
import org.sipfoundry.sipxconfig.feature.LocationFeature;
import org.sipfoundry.sipxconfig.sbc.SbcDeviceManager;

public class BridgeSbcContext implements FeatureProvider, AddressProvider {
    public static final LocationFeature FEATURE = new LocationFeature("sbcBridge");
    public static final AddressType XMLRPC_ADDRESS = new AddressType("sbcBridgeXmlRpc", "https://%s:%d");
    private SbcDeviceManager m_sbcDeviceManager;

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
        return Collections.singleton(XMLRPC_ADDRESS);
    }

    private Address newSbcAddress(BridgeSbc bridge, AddressType type) {
        if (!type.equals(XMLRPC_ADDRESS)) {
            return null;
        }
        return new Address(XMLRPC_ADDRESS, bridge.getAddress(), bridge.getXmlRpcPort());
    }

    @Override
    public Collection<Address> getAvailableAddresses(AddressManager manager, AddressType type, Object requester) {
        if (type == XMLRPC_ADDRESS) {
            if (requester instanceof BridgeSbc) {
                Collections.singleton(newSbcAddress((BridgeSbc) requester, type));
            } else {
                List<Location> locations = manager.getFeatureManager().getLocationsForEnabledFeature(FEATURE);
                List<Address> addresses = new ArrayList<Address>(locations.size());
                List<BridgeSbc> bridges = m_sbcDeviceManager.getBridgeSbcs();
                for (BridgeSbc bridge : bridges) {
                    Location location = bridge.getLocation();
                    if (locations.contains(location)) {
                        addresses.add(newSbcAddress(bridge, type));
                    }
                }
            }
        }
        return null;
    }

    public void setSbcDeviceManager(SbcDeviceManager mgr) {
        m_sbcDeviceManager = mgr;
    }

    @Override
    public void getBundleFeatures(Bundle b) {
        if (b.isRouter()) {
            b.addFeature(FEATURE);
        }
    }
}
