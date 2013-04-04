/**
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
package org.sipfoundry.sipxconfig.networkqueue;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.Writer;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.LinkedList;
import java.util.List;
import java.util.Set;

import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.io.IOUtils;
import org.apache.commons.lang.StringUtils;

import org.sipfoundry.commons.util.PermutationCalculator;
import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.address.AddressProvider;
import org.sipfoundry.sipxconfig.address.AddressType;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigProvider;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigRequest;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigUtils;
import org.sipfoundry.sipxconfig.cfgmgt.KeyValueConfiguration;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
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
import org.sipfoundry.sipxconfig.redis.Redis;
import org.sipfoundry.sipxconfig.setting.BeanWithSettingsDao;
import org.sipfoundry.sipxconfig.snmp.ProcessDefinition;
import org.sipfoundry.sipxconfig.snmp.ProcessProvider;
import org.sipfoundry.sipxconfig.snmp.SnmpManager;

public class NetworkQueueManagerImpl extends SipxHibernateDaoSupport implements NetworkQueueManager, ConfigProvider,
    AddressProvider, FeatureProvider, FirewallProvider, ProcessProvider {

    private static final String ALL_CONTROL_ADDRESSES = "sqa-control-address-all";
    private BeanWithSettingsDao<NetworkQueueSettings> m_settingsDao;

    @Override
    public void replicate(ConfigManager manager, ConfigRequest request) throws IOException {
        if (!request.applies(FEATURE)) {
            return;
        }

        NetworkQueueSettings settings = getSettings();
        Set<Location> locations = request.locations(manager);
        List<String> addresses = new LinkedList<String>();
        for (Location location : locations) {
            addresses.add(location.getAddress());
        }

        List<Location> sqaLocations = manager.getFeatureManager().getLocationsForEnabledFeature(FEATURE);
        Collection<String> sqaAddresses = CollectionUtils.collect(sqaLocations, Location.GET_ADDRESS);
        Collection<String> nonSqaLocations = CollectionUtils.subtract(addresses, sqaAddresses);
        PermutationCalculator<String> permCalc = new PermutationCalculator<String>(addresses);

        int index = 0;
        for (Location location : locations) {
            List<String> sqaCombination =  permCalc.getPermutation(index++, nonSqaLocations.toArray(new String[nonSqaLocations.size()]));

            File dir = manager.getLocationDataDirectory(location);
            boolean enabled = manager.getFeatureManager().isFeatureEnabled(FEATURE, location);

            // CLIENT
            Address queue = manager.getAddressManager().getSingleAddress(CONTROL_ADDRESS, location);
            Writer client = new FileWriter(new File(dir, "sipxsqa-client.ini"));
            try {
                writeClientConfig(client, queue, sqaCombination, enabled);
            } finally {
                IOUtils.closeQuietly(client);
            }

            // SERVER
            ConfigUtils.enableCfengineClass(dir, "sipxsqa.cfdat", enabled, FEATURE.getId());
            if (enabled) {
                Writer server = new FileWriter(new File(dir, "sipxsqa-config.part"));
                try {
                    writeServerConfig(server, settings);
                } finally {
                    IOUtils.closeQuietly(server);
                }
            }
        }
    }

    void writeServerConfig(Writer w, NetworkQueueSettings settings) throws IOException {
        KeyValueConfiguration config = KeyValueConfiguration.equalsSeparated(w);
        config.writeSettings(settings.getSettings().getSetting("sqa-config"));
    }

    void writeClientConfig(Writer w, Address queue, List<String> addresses, boolean isSqaEnabled) throws IOException {
        KeyValueConfiguration config = KeyValueConfiguration.equalsSeparated(w);
        config.write("enabled", queue != null);
        if (queue != null) {
            config.write("sqa-control-port", queue.getCanonicalPort());
            if (isSqaEnabled) {
                config.write("sqa-control-address", queue.getAddress());
            }
        }
        if (!addresses.isEmpty()) {
            config.write(ALL_CONTROL_ADDRESSES, StringUtils.join(addresses, ", "));
        }
    }

    @Override
    public NetworkQueueSettings getSettings() {
        return m_settingsDao.findOrCreateOne();
    }

    @Override
    public void saveSettings(NetworkQueueSettings settings) {
        m_settingsDao.upsert(settings);
    }

    @Override
    public Collection<Address> getAvailableAddresses(AddressManager manager, AddressType type, Location requester) {
        if (!type.equalsAnyOf(CONTROL_ADDRESS, QUEUE_ADDRESS)) {
            return null;
        }

        List<Location> locations = manager.getFeatureManager().getLocationsForEnabledFeature(FEATURE);
        NetworkQueueSettings settings = getSettings();
        int port = (type.equals(CONTROL_ADDRESS) ? settings.getControlPort() : settings.getQueuePort());
        return Location.toAddresses(type, locations, port);
    }

    @Override
    public void featureChangePrecommit(FeatureManager manager, FeatureChangeValidator validator) {
        validator.requiresAtLeastOne(FEATURE, Redis.FEATURE);
    }

    @Override
    public void featureChangePostcommit(FeatureManager manager, FeatureChangeRequest request) {
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
    public void getBundleFeatures(FeatureManager featureManager, Bundle b) {
        if (b == Bundle.CORE_TELEPHONY) {
            b.addFeature(FEATURE);
        }
    }

    @Override
    public Collection<DefaultFirewallRule> getFirewallRules(FirewallManager manager) {
        return DefaultFirewallRule.rules(Arrays.asList(CONTROL_ADDRESS, QUEUE_ADDRESS), FirewallRule.SystemId.CLUSTER);
    }

    public void setSettingsDao(BeanWithSettingsDao<NetworkQueueSettings> settingsDao) {
        m_settingsDao = settingsDao;
    }

    @Override
    public Collection<ProcessDefinition> getProcessDefinitions(SnmpManager manager, Location location) {
        FeatureManager featureManager = manager.getFeatureManager();
        if (!featureManager.isFeatureEnabled(FEATURE, location)) {
            return null;
        }

        ProcessDefinition def = ProcessDefinition.sipx(FEATURE.getId());
        return Collections.singleton(def);
    }
}
