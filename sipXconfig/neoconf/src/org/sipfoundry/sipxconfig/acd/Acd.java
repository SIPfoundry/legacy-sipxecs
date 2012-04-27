/**
 *
 *
 * Copyright (c) 2010 / 2011 eZuce, Inc. All rights reserved.
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
import org.sipfoundry.sipxconfig.firewall.DefaultFirewallRule;
import org.sipfoundry.sipxconfig.firewall.FirewallManager;
import org.sipfoundry.sipxconfig.firewall.FirewallProvider;

public class Acd implements FeatureProvider, AddressProvider, FeatureListener, FirewallProvider {
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
    public Collection<GlobalFeature> getAvailableGlobalFeatures(FeatureManager featureManager) {
        return null;
    }

    @Override
    public Collection<LocationFeature> getAvailableLocationFeatures(FeatureManager featureManager, Location l) {
        return Collections.singleton(FEATURE);
    }

    public boolean isEnabled() {
        return m_featureManager.isFeatureEnabled(FEATURE);
    }

    @Override
    public Collection<Address> getAvailableAddresses(AddressManager manager, AddressType type,
            Location requester) {
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
    public void getBundleFeatures(FeatureManager featureManager, Bundle b) {
        if (b == Bundle.EXPERIMENTAL) {
            b.addFeature(FEATURE);
        }
    }

    @Override
    public Collection<DefaultFirewallRule> getFirewallRules(FirewallManager manager) {
        return DefaultFirewallRule.rules(ADRESSES);
    }
}
