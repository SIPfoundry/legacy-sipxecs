/*
 * Copyright (C) 2012 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.apache;


import java.io.File;
import java.io.IOException;
import java.util.Arrays;
import java.util.Collection;
import java.util.List;
import java.util.Set;

import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.address.AddressProvider;
import org.sipfoundry.sipxconfig.address.AddressType;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigProvider;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigRequest;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigUtils;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.dns.DnsManager;
import org.sipfoundry.sipxconfig.dns.DnsProvider;
import org.sipfoundry.sipxconfig.dns.ResourceRecords;
import org.sipfoundry.sipxconfig.firewall.DefaultFirewallRule;
import org.sipfoundry.sipxconfig.firewall.FirewallManager;
import org.sipfoundry.sipxconfig.firewall.FirewallProvider;
import org.sipfoundry.sipxconfig.firewall.FirewallRule;
import org.sipfoundry.sipxconfig.setup.SetupListener;
import org.sipfoundry.sipxconfig.setup.SetupManager;

public class ApacheManagerImpl extends SipxHibernateDaoSupport<Object> implements ApacheManager, ConfigProvider,
        SetupListener, FirewallProvider, AddressProvider, DnsProvider {
    private LocationsManager m_locationsManager;

    @Override
    public void replicate(ConfigManager manager, ConfigRequest request) throws IOException {
        if (!request.applies(FEATURE)) {
            return;
        }

        Set<Location> locations = request.locations(manager);
        for (Location l : locations) {
            boolean enabled = manager.getFeatureManager().isFeatureEnabled(FEATURE, l);
            File dir = manager.getLocationDataDirectory(l);
            ConfigUtils.enableCfengineClass(dir, "apache.cfdat", enabled, "apache");
        }
    }

    @Override
    public boolean setup(SetupManager manager) {
        if (manager.isFalse(FEATURE.getId())) {
            Location primary = m_locationsManager.getPrimaryLocation();
            manager.getFeatureManager().enableLocationFeature(FEATURE, primary, true);
            manager.setTrue(FEATURE.getId());
        }
        return true;
    }

    @Override
    public void avoidCheckstyleError() {
    }

    @Override
    public Collection<DefaultFirewallRule> getFirewallRules(FirewallManager manager) {
        return DefaultFirewallRule.rules(Arrays.asList(HTTP_ADDRESS, HTTP_STATIC_ADDRESS, HTTPS_ADDRESS),
               FirewallRule.SystemId.PUBLIC);
    }

    @Override
    public Collection<Address> getAvailableAddresses(AddressManager manager, AddressType type, Location requester) {
        if (!type.equalsAnyOf(HTTP_ADDRESS, HTTP_STATIC_ADDRESS, HTTPS_ADDRESS)) {
            return null;
        }

        List<Location> locations = manager.getFeatureManager().getLocationsForEnabledFeature(FEATURE);
        return Location.toAddresses(type, locations);
    }

    @Override
    public Address getAddress(DnsManager manager, AddressType t, Collection<Address> addresses, Location whoIsAsking) {
        if (!t.equals(HTTPS_ADDRESS)) {
            return null;
        }

        return new Address(t, m_locationsManager.getPrimaryLocation().getFqdn());
    }

    @Override
    public List<ResourceRecords> getResourceRecords(DnsManager manager, Location whoIsAsking) {
        return null;
    }

    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }
}
