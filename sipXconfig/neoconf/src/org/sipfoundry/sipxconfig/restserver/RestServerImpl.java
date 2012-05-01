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
package org.sipfoundry.sipxconfig.restserver;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.List;

import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.address.AddressProvider;
import org.sipfoundry.sipxconfig.address.AddressType;
import org.sipfoundry.sipxconfig.cdr.CdrManager;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.feature.Bundle;
import org.sipfoundry.sipxconfig.feature.FeatureChangeRequest;
import org.sipfoundry.sipxconfig.feature.FeatureChangeValidator;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.feature.FeatureProvider;
import org.sipfoundry.sipxconfig.feature.GlobalFeature;
import org.sipfoundry.sipxconfig.feature.LocationFeature;
import org.sipfoundry.sipxconfig.firewall.DefaultFirewallRule;
import org.sipfoundry.sipxconfig.firewall.FirewallManager;
import org.sipfoundry.sipxconfig.firewall.FirewallProvider;
import org.sipfoundry.sipxconfig.firewall.FirewallRule;
import org.sipfoundry.sipxconfig.proxy.ProxyManager;
import org.sipfoundry.sipxconfig.setting.BeanWithSettingsDao;
import org.sipfoundry.sipxconfig.snmp.ProcessDefinition;
import org.sipfoundry.sipxconfig.snmp.ProcessProvider;
import org.sipfoundry.sipxconfig.snmp.SnmpManager;

public class RestServerImpl implements FeatureProvider, AddressProvider, RestServer, ProcessProvider, FirewallProvider {
    private static final Collection<AddressType> ADDRESSES = Arrays.asList(HTTP_API, PUBLIC_HTTP_API, SIP_TCP);
    private BeanWithSettingsDao<RestServerSettings> m_settingsDao;

    @Override
    public RestServerSettings getSettings() {
        return m_settingsDao.findOrCreateOne();
    }

    @Override
    public void saveSettings(RestServerSettings settings) {
        m_settingsDao.upsert(settings);
    }

    @Override
    public Collection<GlobalFeature> getAvailableGlobalFeatures(FeatureManager featureManager) {
        return null;
    }

    @Override
    public Collection<LocationFeature> getAvailableLocationFeatures(FeatureManager featureManager, Location l) {
        return Collections.singleton(FEATURE);
    }

    @Override
    public Collection<Address> getAvailableAddresses(AddressManager manager, AddressType type,
            Location requester) {
        if (!ADDRESSES.contains(type)) {
            return null;
        }
        RestServerSettings settings = getSettings();
        Collection<Location> locations = manager.getFeatureManager().getLocationsForEnabledFeature(FEATURE);
        List<Address> addresses = new ArrayList<Address>(locations.size());
        for (Location location : locations) {
            Address address = null;
            if (type.equals(HTTP_API)) {
                address = new Address(HTTP_API, location.getAddress(), settings.getHttpPort());
            } else if (type.equals(PUBLIC_HTTP_API)) {
                address = new Address(PUBLIC_HTTP_API, location.getAddress(), settings.getPublicHttpPort());
            } else if (type.equals(SIP_TCP)) {
                address = new Address(SIP_TCP, location.getAddress(), settings.getSipPort());
            }
            addresses.add(address);
        }

        return addresses;
    }

    public void setSettingsDao(BeanWithSettingsDao<RestServerSettings> settingsDao) {
        m_settingsDao = settingsDao;
    }

    @Override
    public Collection<ProcessDefinition> getProcessDefinitions(SnmpManager manager, Location location) {
        boolean enabled = manager.getFeatureManager().isFeatureEnabled(FEATURE, location);
        return (enabled ? Collections.singleton(new ProcessDefinition("sipxrest", ".*\\s-Dprocname=sipxrest\\s.*"))
                : null);
    }

    @Override
    public void getBundleFeatures(FeatureManager featureManager, Bundle b) {
        if (b == Bundle.CORE_TELEPHONY) {
            b.addFeature(FEATURE);
        }
    }

    @Override
    public Collection<DefaultFirewallRule> getFirewallRules(FirewallManager manager) {
        List<DefaultFirewallRule> rules = DefaultFirewallRule.rules(Arrays.asList(HTTP_API, SIP_TCP));

        // this should probably proxies thru apache, if so remove this line
        rules.add(new DefaultFirewallRule(PUBLIC_HTTP_API, FirewallRule.SystemId.PUBLIC));
        return rules;
    }

    @Override
    public void featureChangePrecommit(FeatureManager manager, FeatureChangeValidator validator) {
        validator.requiresAtLeastOne(FEATURE, CdrManager.FEATURE);
        validator.requiresAtLeastOne(FEATURE, ProxyManager.FEATURE);
    }

    @Override
    public void featureChangePostcommit(FeatureManager manager, FeatureChangeRequest request) {
    }
}
