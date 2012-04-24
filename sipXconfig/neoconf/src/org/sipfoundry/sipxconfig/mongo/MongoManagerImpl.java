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
package org.sipfoundry.sipxconfig.mongo;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;

import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.address.AddressProvider;
import org.sipfoundry.sipxconfig.address.AddressType;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.feature.Bundle;
import org.sipfoundry.sipxconfig.feature.BundleConstraint;
import org.sipfoundry.sipxconfig.feature.FeatureProvider;
import org.sipfoundry.sipxconfig.feature.GlobalFeature;
import org.sipfoundry.sipxconfig.feature.LocationFeature;
import org.sipfoundry.sipxconfig.firewall.DefaultFirewallRule;
import org.sipfoundry.sipxconfig.firewall.FirewallManager;
import org.sipfoundry.sipxconfig.firewall.FirewallProvider;
import org.sipfoundry.sipxconfig.setting.BeanWithSettingsDao;
import org.sipfoundry.sipxconfig.setup.SetupListener;
import org.sipfoundry.sipxconfig.setup.SetupManager;
import org.sipfoundry.sipxconfig.snmp.ProcessDefinition;
import org.sipfoundry.sipxconfig.snmp.ProcessProvider;
import org.sipfoundry.sipxconfig.snmp.SnmpManager;

public class MongoManagerImpl implements AddressProvider, FeatureProvider, MongoManager, ProcessProvider,
    SetupListener, FirewallProvider {
    private BeanWithSettingsDao<MongoSettings> m_settingsDao;

    public MongoSettings getSettings() {
        return m_settingsDao.findOrCreateOne();
    }

    public void saveSettings(MongoSettings settings) {
        m_settingsDao.upsert(settings);
    }

    @Override
    public Collection<DefaultFirewallRule> getFirewallRules(FirewallManager manager) {
        return Collections.singleton(new DefaultFirewallRule(ADDRESS_ID));
    }

    @Override
    public Collection<Address> getAvailableAddresses(AddressManager manager, AddressType type,
            Location requester) {
        if (!type.equalsAnyOf(ADDRESS_ID)) {
            return null;
        }
        Collection<Location> locations = manager.getFeatureManager().getLocationsForEnabledFeature(FEATURE_ID);
        Collection<Address> addresses = new ArrayList<Address>(locations.size());
        for (Location location : locations) {
            addresses.add(new Address(ADDRESS_ID, location.getAddress(), MongoSettings.SERVER_PORT));
        }
        return addresses;
    }

    @Override
    public Collection<GlobalFeature> getAvailableGlobalFeatures() {
        return null;
    }

    @Override
    public Collection<LocationFeature> getAvailableLocationFeatures(Location l) {
        if (l.isPrimary()) {
            return Collections.singleton(ARBITER_FEATURE);
        }
        return Arrays.asList(FEATURE_ID, ARBITER_FEATURE);
    }

    public void setSettingsDao(BeanWithSettingsDao<MongoSettings> settingsDao) {
        m_settingsDao = settingsDao;
    }

    @Override
    public Collection<ProcessDefinition> getProcessDefinitions(SnmpManager manager, Location location) {
        Collection<ProcessDefinition> procs = new ArrayList<ProcessDefinition>(2);
        if (manager.getFeatureManager().isFeatureEnabled(FEATURE_ID, location) || location.isPrimary()) {
            procs.add(new ProcessDefinition("mongod", ".*/mongod.*-f.*/mongod.conf"));
        }
        if (manager.getFeatureManager().isFeatureEnabled(ARBITER_FEATURE, location)) {
            procs.add(new ProcessDefinition("mongoArbiter", ".*/mongod.*-f.*/mongod-arbiter.conf"));
        }
        return procs;
    }

    @Override
    public void getBundleFeatures(Bundle b) {
        if (b.isBasic()) {
            b.addFeature(FEATURE_ID);
            b.addFeature(ARBITER_FEATURE, BundleConstraint.SINGLE_LOCATION);
        }
    }

    @Override
    public void setup(SetupManager manager) {
        if (!manager.isSetup(FEATURE_ID.getId())) {
            Location primary = manager.getConfigManager().getLocationManager().getPrimaryLocation();
            manager.getFeatureManager().enableLocationFeature(FEATURE_ID, primary, true);
            manager.setSetup(FEATURE_ID.getId());
        }
    }
}
