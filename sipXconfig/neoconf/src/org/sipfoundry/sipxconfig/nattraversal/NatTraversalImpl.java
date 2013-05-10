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
package org.sipfoundry.sipxconfig.nattraversal;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.List;

import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.address.AddressProvider;
import org.sipfoundry.sipxconfig.address.AddressType;
import org.sipfoundry.sipxconfig.alarm.AlarmDefinition;
import org.sipfoundry.sipxconfig.alarm.AlarmProvider;
import org.sipfoundry.sipxconfig.alarm.AlarmServerManager;
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

public class NatTraversalImpl implements NatTraversal, FeatureProvider, ProcessProvider, FirewallProvider,
        AddressProvider, AlarmProvider {
    private BeanWithSettingsDao<NatSettings> m_settingsDao;

    public NatSettings getSettings() {
        return m_settingsDao.findOrCreateOne();
    }

    public void saveSettings(NatSettings settings) {
        m_settingsDao.upsert(settings);
    }

    public void setSettingsDao(BeanWithSettingsDao<NatSettings> settingsDao) {
        m_settingsDao = settingsDao;
    }

    @Override
    public Collection<GlobalFeature> getAvailableGlobalFeatures(FeatureManager featureManager) {
        return Collections.singleton(FEATURE);
    }

    @Override
    public Collection<LocationFeature> getAvailableLocationFeatures(FeatureManager featureManager, Location l) {
        return null;
    }

    @Override
    public Collection<ProcessDefinition> getProcessDefinitions(SnmpManager manager, Location location) {
        boolean relayEnabled = manager.getFeatureManager().isFeatureEnabled(FEATURE);
        boolean proxyEnabled = manager.getFeatureManager().isFeatureEnabled(ProxyManager.FEATURE, location);
        return (relayEnabled && proxyEnabled ? Collections.singleton(ProcessDefinition.sipxByRegex("sipxrelay",
                ".*\\s-Dprocname=sipxrelay\\s.*")) : null);
    }

    @Override
    public void getBundleFeatures(FeatureManager featureManager, Bundle b) {
        // No sense showing this, feature is controlled by proxy on/off status
        // if (b == Bundle.CORE_TELEPHONY) {
        // // NAT traversal as basic bundle is debatable but proxy requires it ATM AFAIU
        // b.addFeature(FEATURE);
        // }
    }

    @Override
    public Collection<Address> getAvailableAddresses(AddressManager manager, AddressType type, Location requester) {
        if (!(type.equals(RELAY_RTP) || type.equals(RELAY_RPC))) {
            return null;
        }

        boolean relayEnabled = manager.getFeatureManager().isFeatureEnabled(FEATURE);
        if (!relayEnabled) {
            return null;
        }

        List<Location> locations = manager.getFeatureManager().getLocationsForEnabledFeature(ProxyManager.FEATURE);
        List<Address> addresses = new ArrayList<Address>(locations.size());

        if (type.equals(RELAY_RTP)) {
            for (Location location : locations) {
                Address a = new Address(type, location.getAddress(), location.getStartRtpPort());
                a.setEndPort(location.getStopRtpPort());
                addresses.add(a);
            }
        }
        if (type.equals(RELAY_RPC)) {
            for (Location location : locations) {
                Address a = new Address(type, location.getAddress(), getSettings().getXmlRpcPort());
                addresses.add(a);
            }
        }
        return addresses;
    }

    @Override
    public Collection<DefaultFirewallRule> getFirewallRules(FirewallManager manager) {
        return Arrays.asList(new DefaultFirewallRule(RELAY_RTP, FirewallRule.SystemId.PUBLIC, true),
                new DefaultFirewallRule(RELAY_RPC, FirewallRule.SystemId.CLUSTER, false));
    }

    @Override
    public void featureChangePrecommit(FeatureManager manager, FeatureChangeValidator validator) {
        // See ProxyManagerImpl for details
    }

    @Override
    public void featureChangePostcommit(FeatureManager manager, FeatureChangeRequest request) {
        if (request.hasChanged(FEATURE)) {
            NatSettings settings = getSettings();
            if (!settings.isBehindNat()) {
                boolean enable = request.getAllNewlyEnabledFeatures().contains(FEATURE);
                settings.setBehindNat(enable);
                saveSettings(settings);
            }
        }
    }

    @Override
    public Collection<AlarmDefinition> getAvailableAlarms(AlarmServerManager manager) {
        if (!manager.getFeatureManager().isFeatureEnabled(FEATURE)) {
            return null;
        }
        String[] ids = new String[] {
            "MEDIA_RELAY_STUN_FAILURE", "MEDIA_RELAY_STUN_RECOVERY", "MEDIA_RELAY_STUN_ADDRESS_ERROR",
            "MEDIA_RELAY_STRAY_PACKET"
        };
        return AlarmDefinition.asArray(ids);
    }
}
