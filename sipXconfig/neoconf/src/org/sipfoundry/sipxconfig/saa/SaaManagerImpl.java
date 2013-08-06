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
package org.sipfoundry.sipxconfig.saa;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;

import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.address.AddressProvider;
import org.sipfoundry.sipxconfig.address.AddressType;
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
import org.sipfoundry.sipxconfig.proxy.ProxyManager;
import org.sipfoundry.sipxconfig.setting.BeanWithSettingsDao;
import org.sipfoundry.sipxconfig.snmp.ProcessDefinition;
import org.sipfoundry.sipxconfig.snmp.ProcessProvider;
import org.sipfoundry.sipxconfig.snmp.SnmpManager;

public class SaaManagerImpl implements FeatureProvider, SaaManager, ProcessProvider, FirewallProvider,
        AddressProvider {
    private BeanWithSettingsDao<SaaSettings> m_settingsDao;

    @Override
    public SaaSettings getSettings() {
        return m_settingsDao.findOrCreateOne();
    }

    @Override
    public void saveSettings(SaaSettings settings) {
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

    public void setSettingsDao(BeanWithSettingsDao<SaaSettings> settingsDao) {
        m_settingsDao = settingsDao;
    }

    @Override
    public Collection<ProcessDefinition> getProcessDefinitions(SnmpManager manager, Location location) {
        boolean enabled = manager.getFeatureManager().isFeatureEnabled(FEATURE, location);
        return (enabled ? Collections.singleton(ProcessDefinition.sipx("sipxsaa")) : null);
    }

    @Override
    public void getBundleFeatures(FeatureManager featureManager, Bundle b) {
        if (b == Bundle.CORE_TELEPHONY) {
            b.addFeature(FEATURE);
        }
    }

    @Override
    public void featureChangePrecommit(FeatureManager manager, FeatureChangeValidator validator) {
        validator.requiresAtLeastOne(FEATURE, ProxyManager.FEATURE);
        validator.singleLocationOnly(FEATURE);
    }

    @Override
    public void featureChangePostcommit(FeatureManager manager, FeatureChangeRequest request) {
    }

    @Override
    public Collection<DefaultFirewallRule> getFirewallRules(FirewallManager manager) {
        return DEFAULT_RULES;
    }

    @Override
    public Collection<Address> getAvailableAddresses(AddressManager manager, AddressType type, Location requester) {
        Collection<Address> addresses = new ArrayList<Address>();

        if (SUPPORTED_ADDRESS_TYPES.contains(type)) {
            Collection<Location> locations = manager.getFeatureManager().getLocationsForEnabledFeature(FEATURE);
            for (Location location : locations) {
                if (SAA_TCP.equals(type)) {
                    addresses.add(new Address(SAA_TCP, location.getAddress(), getSettings().getTcpPort()));
                } else if (SAA_UDP.equals(type)) {
                    addresses.add(new Address(SAA_UDP, location.getAddress(), getSettings().getUdpPort()));
                }
            }
        }

        return addresses;
    }
}
