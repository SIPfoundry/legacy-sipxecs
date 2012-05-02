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
import java.util.List;
import java.util.Set;

import org.apache.commons.io.IOUtils;
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
import org.sipfoundry.sipxconfig.feature.BundleConstraint;
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
import org.sipfoundry.sipxconfig.setting.BeanWithSettingsDao;

public class NetworkQueueManagerImpl extends SipxHibernateDaoSupport implements NetworkQueueManager, ConfigProvider,
    AddressProvider, FeatureProvider, FirewallProvider {
    private BeanWithSettingsDao<NetworkQueueSettings> m_settingsDao;

    @Override
    public void replicate(ConfigManager manager, ConfigRequest request) throws IOException {
        if (!request.applies(FEATURE)) {
            return;
        }

        NetworkQueueSettings settings = getSettings();
        Set<Location> locations = request.locations(manager);
        for (Location location : locations) {
            File dir = manager.getLocationDataDirectory(location);
            boolean enabled = manager.getFeatureManager().isFeatureEnabled(FEATURE, location);
            ConfigUtils.enableCfengineClass(dir, "sipxsqa.cfdat", enabled, "sipxsqa");
            if (!enabled) {
                continue;
            }
            Writer config = new FileWriter(new File(dir, "sipxsqa-config.part"));
            try {
                writeConfig(config, settings);
            } finally {
                IOUtils.closeQuietly(config);
            }
        }
    }

    void writeConfig(Writer w, NetworkQueueSettings settings) throws IOException {
        KeyValueConfiguration config = KeyValueConfiguration.colonSeparated(w);
        config.write(settings.getSettings().getSetting("sqa-config"));
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
        // redis?
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
        if (b == Bundle.EXPERIMENTAL) {
            b.addFeature(FEATURE, BundleConstraint.SINGLE_LOCATION);
        }
    }

    @Override
    public Collection<DefaultFirewallRule> getFirewallRules(FirewallManager manager) {
        return DefaultFirewallRule.rules(Arrays.asList(CONTROL_ADDRESS, QUEUE_ADDRESS), FirewallRule.SystemId.CLUSTER);
    }

    public void setSettingsDao(BeanWithSettingsDao<NetworkQueueSettings> settingsDao) {
        m_settingsDao = settingsDao;
    }
}
