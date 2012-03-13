/*
 * Copyright (C) 2012 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
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
