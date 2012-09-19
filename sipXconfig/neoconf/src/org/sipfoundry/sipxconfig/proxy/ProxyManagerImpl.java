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
package org.sipfoundry.sipxconfig.proxy;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;

import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.address.AddressProvider;
import org.sipfoundry.sipxconfig.address.AddressType;
import org.sipfoundry.sipxconfig.alarm.AlarmDefinition;
import org.sipfoundry.sipxconfig.alarm.AlarmProvider;
import org.sipfoundry.sipxconfig.alarm.AlarmServerManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.dialplan.DialPlanContext;
import org.sipfoundry.sipxconfig.dns.DnsManager;
import org.sipfoundry.sipxconfig.feature.Bundle;
import org.sipfoundry.sipxconfig.feature.FeatureChangeRequest;
import org.sipfoundry.sipxconfig.feature.FeatureChangeValidator;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.feature.FeatureProvider;
import org.sipfoundry.sipxconfig.feature.GlobalFeature;
import org.sipfoundry.sipxconfig.feature.InvalidChange;
import org.sipfoundry.sipxconfig.feature.LocationFeature;
import org.sipfoundry.sipxconfig.firewall.DefaultFirewallRule;
import org.sipfoundry.sipxconfig.firewall.FirewallManager;
import org.sipfoundry.sipxconfig.firewall.FirewallProvider;
import org.sipfoundry.sipxconfig.firewall.FirewallRule;
import org.sipfoundry.sipxconfig.nattraversal.NatTraversal;
import org.sipfoundry.sipxconfig.registrar.Registrar;
import org.sipfoundry.sipxconfig.setting.BeanWithSettingsDao;
import org.sipfoundry.sipxconfig.snmp.ProcessDefinition;
import org.sipfoundry.sipxconfig.snmp.ProcessProvider;
import org.sipfoundry.sipxconfig.snmp.SnmpManager;

public class ProxyManagerImpl implements ProxyManager, FeatureProvider, AddressProvider, ProcessProvider,
    AlarmProvider, FirewallProvider {

    private static final Collection<AddressType> ADDRESS_TYPES = Arrays.asList(new AddressType[] {
        TCP_ADDRESS, UDP_ADDRESS, TLS_ADDRESS
    });
    private static final String PROCESS = "sipxproxy";
    private FeatureManager m_featureManager;
    private BeanWithSettingsDao<ProxySettings> m_settingsDao;
    private ConfigManager m_configManager;

    @Override
    public Collection<GlobalFeature> getAvailableGlobalFeatures(FeatureManager featureManager) {
        return null;
    }

    @Override
    public Collection<LocationFeature> getAvailableLocationFeatures(FeatureManager featureManager, Location l) {
        return Collections.singleton(FEATURE);
    }

    public ProxySettings getSettings() {
        return m_settingsDao.findOrCreateOne();
    }

    public void saveSettings(ProxySettings settings) {
        m_settingsDao.upsert(settings);
    }

    @Override
    public Collection<DefaultFirewallRule> getFirewallRules(FirewallManager manager) {
        return DefaultFirewallRule.rules(ADDRESS_TYPES, FirewallRule.SystemId.PUBLIC);
    }

    @Override
    public Collection<Address> getAvailableAddresses(AddressManager manager, AddressType type,
            Location requester) {
        if (!ADDRESS_TYPES.contains(type)) {
            return null;
        }
        Collection<Address> addresses = null;
        Collection<Location> locations = m_featureManager.getLocationsForEnabledFeature(FEATURE);
        addresses = new ArrayList<Address>(locations.size());
        for (Location location : locations) {
            Address address = null;
            if (type.equals(TCP_ADDRESS)) {
                address = new Address(TCP_ADDRESS, location.getAddress(), 5060);
            } else if (type.equals(UDP_ADDRESS)) {
                address = new Address(UDP_ADDRESS, location.getAddress(), 5060);
            } else if (type.equals(TLS_ADDRESS)) {
                address = new Address(TCP_ADDRESS, location.getAddress(), 5061);
            }
            addresses.add(address);
        }

        return addresses;
    }

    public void setFeatureManager(FeatureManager featureManager) {
        m_featureManager = featureManager;
    }

    public void setSettingsDao(BeanWithSettingsDao<ProxySettings> settingsDao) {
        m_settingsDao = settingsDao;
    }

    @Override
    public Collection<ProcessDefinition> getProcessDefinitions(SnmpManager manager, Location location) {
        if (!m_featureManager.isFeatureEnabled(FEATURE, location)) {
            return null;
        }
        ProcessDefinition def = ProcessDefinition.sipx(PROCESS, PROCESS);
        return Collections.singleton(def);
    }

    public void setConfigManager(ConfigManager configManager) {
        m_configManager = configManager;
    }

    @Override
    public Collection<AlarmDefinition> getAvailableAlarms(AlarmServerManager manager) {
        if (!manager.getFeatureManager().isFeatureEnabled(FEATURE)) {
            return null;
        }
        String[] ids = new String[] {
            "PROXY_FAILED_TO_INITIALIZE_MEDIA_RELAY", "PROXY_EMERG_NUMBER_DIALED",
            "PROXY_RAN_OUT_OF_MEDIA_RELAY_SESSIONS", "PROXY_MEDIA_RELAY_RECONNECTED",
            "PROXY_MEDIA_RELAY_RESET_DETECTED_RECONNECTING", "PROXY_MEDIA_RELAY_RESET_DETECTED_RECONNECTED",
            "PROXY_LOST_CONTACT_WITH_MEDIA_RELAY"
        };
        return AlarmDefinition.asArray(ids);
    }

    @Override
    public void getBundleFeatures(FeatureManager featureManager, Bundle b) {
        if (b == Bundle.CORE_TELEPHONY) {
            b.addFeature(FEATURE);
        }
    }

    @Override
    public void featureChangePrecommit(FeatureManager manager, FeatureChangeValidator validator) {
        // make sure nat and proxy are on/off together
        boolean proxyOn = validator.isEnabledSomewhere(FEATURE);
        if (validator.isEnabledSomewhere(NatTraversal.FEATURE) != proxyOn) {
            validator.getRequest().enableFeature(NatTraversal.FEATURE, proxyOn);
        }

        // Do not auto resolve to avoid circular dependency
        if (validator.isEnabledSomewhere(FEATURE) && !validator.isEnabledSomewhere(Registrar.FEATURE)) {
            InvalidChange requires = InvalidChange.requires(FEATURE, Registrar.FEATURE);
            requires.setAllowAutoResolve(false);
            validator.getInvalidChanges().add(requires);
        }
    }

    @Override
    public void featureChangePostcommit(FeatureManager manager, FeatureChangeRequest request) {
        if (request.hasChanged(FEATURE)) {
            m_configManager.configureEverywhere(DnsManager.FEATURE, DialPlanContext.FEATURE);
        }
    }
}
