/**
 * Copyright (c) 2014 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.sipxedgerpc;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.Writer;
import java.util.ArrayList;
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
import org.sipfoundry.sipxconfig.setting.BeanWithSettingsDao;
import org.sipfoundry.sipxconfig.snmp.ProcessDefinition;
import org.sipfoundry.sipxconfig.snmp.ProcessProvider;
import org.sipfoundry.sipxconfig.snmp.SnmpManager;

public class SipxedgerpcManagerImpl implements SipxedgerpcManager, ConfigProvider, FeatureProvider,
        FirewallProvider, ProcessProvider, AddressProvider {
    private static final Collection<AddressType> ADDRESSES = Arrays.asList(new AddressType[] {
        HTTP_ADDRESS, MONIT_HTTP_ADDRESS
    });
    private BeanWithSettingsDao<SipxedgerpcManagerSettings> m_settingsDao;

    @Override
    public void replicate(ConfigManager manager, ConfigRequest request) throws IOException {
        if (!request.applies(FEATURE)) {
            return;
        }
        FeatureManager featureManager = manager.getFeatureManager();
        SipxedgerpcManagerSettings settings = getSettings();
        Set<Location> locations = request.locations(manager);
        for (Location location : locations) {
            File dir = manager.getLocationDataDirectory(location);
            boolean enabled = featureManager.isFeatureEnabled(FEATURE, location);

            ConfigUtils.enableCfengineClass(dir, "sipxedgerpc.cfdat", enabled, "sipxedgerpc");
            if (!enabled) {
                continue;
            }
            Writer client = new FileWriter(new File(dir, "sipxedgerpc-config.part"));
            try {
                writeConfig(client, settings);
            } finally {
                IOUtils.closeQuietly(client);
            }
        }
    }

    protected static void writeConfig(Writer w, SipxedgerpcManagerSettings settings) throws IOException {
        KeyValueConfiguration config = KeyValueConfiguration.equalsSeparated(w);
        config.write("http-port", settings.getHttpPort());
        config.write("log-level", settings.getLogLevel());
    }

    @Override
    public SipxedgerpcManagerSettings getSettings() {
        return m_settingsDao.findOrCreateOne();
    }

    @Override
    public void saveSettings(SipxedgerpcManagerSettings settings) {
        m_settingsDao.upsert(settings);
    }

    @Override
    public Collection<Address> getAvailableAddresses(AddressManager manager, AddressType type, Location requester) {
        if (!ADDRESSES.contains(type)) {
            return null;
        }
        List<Address> addresses = new ArrayList<Address>();
        List<Location> locations = manager.getFeatureManager().getLocationsForEnabledFeature(FEATURE);
        SipxedgerpcManagerSettings settings = getSettings();
        if (type.equals(HTTP_ADDRESS)) {
            addresses.addAll(Location.toAddresses(type, locations, settings.getHttpPort()));
        } else if (type.equals(MONIT_HTTP_ADDRESS)) {
            addresses.addAll(Location.toAddresses(type, locations, settings.getMonitHttpPort()));
        }
        return addresses;
    }

    @Override
    public void featureChangePrecommit(FeatureManager manager, FeatureChangeValidator validator) {
        validator.singleLocationOnly(FEATURE);
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
        return DefaultFirewallRule.rules(Arrays.asList(HTTP_ADDRESS, MONIT_HTTP_ADDRESS),
                FirewallRule.SystemId.PUBLIC);
    }

    public void setSettingsDao(BeanWithSettingsDao<SipxedgerpcManagerSettings> settingsDao) {
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

    @Override
    public void method() {

    }
}
