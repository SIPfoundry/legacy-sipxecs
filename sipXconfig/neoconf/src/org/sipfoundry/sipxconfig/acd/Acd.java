/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.acd;

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
import org.sipfoundry.sipxconfig.feature.Bundle;
import org.sipfoundry.sipxconfig.feature.FeatureListener;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.feature.FeatureProvider;
import org.sipfoundry.sipxconfig.feature.GlobalFeature;
import org.sipfoundry.sipxconfig.feature.LocationFeature;

public class Acd implements FeatureProvider, AddressProvider, FeatureListener {
    public static final LocationFeature FEATURE = new LocationFeature("acd");
    public static final AddressType CONFIG_ADDRESS = new AddressType("acdConfig");
    public static final AddressType MONITOR_ADDRESS = new AddressType("acdMonitor");
    public static final AddressType TCP_SIP_ADDRESS = new AddressType("acdSipTcp");
    public static final AddressType UDP_SIP_ADDRESS = new AddressType("acdSipUdp");
    public static final AddressType TLS_SIP_ADDRESS = new AddressType("acdSipTls");
    private static final Collection<AddressType> ADRESSES = Arrays.asList(CONFIG_ADDRESS, MONITOR_ADDRESS,
            TCP_SIP_ADDRESS, UDP_SIP_ADDRESS, TLS_SIP_ADDRESS);
    private FeatureManager m_featureManager;
    private AcdContext m_acdContext;

    @Override
    public Collection<GlobalFeature> getAvailableGlobalFeatures() {
        return null;
    }

    @Override
    public Collection<LocationFeature> getAvailableLocationFeatures(Location l) {
        return Collections.singleton(FEATURE);
    }

    public boolean isEnabled() {
        return m_featureManager.isFeatureEnabled(FEATURE);
    }

    @Override
    public Collection<AddressType> getSupportedAddressTypes(AddressManager manager) {
        return ADRESSES;
    }

    @Override
    public Collection<Address> getAvailableAddresses(AddressManager manager, AddressType type,
            Object requester) {
        List<Address> addresses = null;
        if (ADRESSES.contains(type)) {
            m_acdContext.getServers();
            Collection<Location> locations = m_featureManager.getLocationsForEnabledFeature(FEATURE);
            addresses = new ArrayList<Address>();
            for (Location location : locations) {
                AcdServer server = m_acdContext.getAcdServerForLocationId(location.getId());
                Address address = null;
                if (type.equals(CONFIG_ADDRESS)) {
                    address = new Address(CONFIG_ADDRESS, location.getAddress(), server.getPort());
                } else if (type.equals(TCP_SIP_ADDRESS)) {
                    address.setPort(server.getSipPort());
                } else if (type.equals(UDP_SIP_ADDRESS)) {
                    address.setPort(server.getSipPort());
                } else if (type.equals(TLS_SIP_ADDRESS)) {
                    address.setPort(server.getTlsPort());
                } else if (type.equals(MONITOR_ADDRESS)) {
                    address.setPort(server.getMonitorPort());
                }
                addresses.add(address);
            }
        }
        return addresses;
    }

    @Override
    public void enableLocationFeature(FeatureManager manager, FeatureEvent event, LocationFeature feature,
            Location location) {
        if (!Acd.FEATURE.equals(feature)) {
            return;
        }
        if (event == FeatureEvent.PRE_ENABLE) {
            m_acdContext.addNewServer(location);
        } else if (event == FeatureEvent.POST_DISABLE) {
            List<AcdServer> servers = m_acdContext.getServers();
            m_acdContext.removeServers(servers);
        }
    }

    @Override
    public void enableGlobalFeature(FeatureManager manager, FeatureEvent event, GlobalFeature feature) {
    }

    public void setFeatureManager(FeatureManager featureManager) {
        m_featureManager = featureManager;
    }

    public void setAcdContext(AcdContext acdContext) {
        m_acdContext = acdContext;
    }

    @Override
    public void getBundleFeatures(Bundle b) {
    }
}
