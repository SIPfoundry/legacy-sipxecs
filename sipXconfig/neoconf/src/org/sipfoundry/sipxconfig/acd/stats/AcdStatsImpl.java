/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.acd.stats;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.List;

import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.address.AddressProvider;
import org.sipfoundry.sipxconfig.address.AddressType;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.feature.Bundle;
import org.sipfoundry.sipxconfig.feature.FeatureProvider;
import org.sipfoundry.sipxconfig.feature.GlobalFeature;
import org.sipfoundry.sipxconfig.feature.LocationFeature;
import org.sipfoundry.sipxconfig.setting.BeanWithSettingsDao;
import org.sipfoundry.sipxconfig.snmp.ProcessDefinition;
import org.sipfoundry.sipxconfig.snmp.ProcessProvider;
import org.sipfoundry.sipxconfig.snmp.SnmpManager;
import org.springframework.beans.factory.annotation.Required;

public class AcdStatsImpl implements AcdStats, FeatureProvider, AddressProvider, ProcessProvider {
    private BeanWithSettingsDao<AcdStatsSettings> m_settingsDao;

    public AcdStatsSettings getSettings() {
        return m_settingsDao.findOrCreateOne();
    }

    public void saveSettings(AcdStatsSettings settings) {
        m_settingsDao.upsert(settings);
    }

    @Override
    public Collection<GlobalFeature> getAvailableGlobalFeatures() {
        return null;
    }

    @Override
    public Collection<LocationFeature> getAvailableLocationFeatures(Location l) {
        return Collections.singleton(FEATURE);
    }

    @Override
    public Collection<AddressType> getSupportedAddressTypes(AddressManager manager) {
        return Collections.singleton(API_ADDRESS);
    }

    @Override
    public Collection<Address> getAvailableAddresses(AddressManager manager, AddressType type,
            Object requester) {
        if (!type.equals(API_ADDRESS)) {
            return null;
        }

        List<Address> addresses = null;
        List<Location> locations = manager.getFeatureManager().getLocationsForEnabledFeature(FEATURE);
        if (locations != null && !locations.isEmpty()) {
            AcdStatsSettings settings = getSettings();
            addresses = new ArrayList<Address>(locations.size());
            for (Location location : locations) {
                addresses.add(new Address(API_ADDRESS, location.getAddress(), settings.getAcdStatsPort()));
            }
        }
        return addresses;
    }

    @Required
    public void setSettingsDao(BeanWithSettingsDao settingsDao) {
        m_settingsDao = settingsDao;
    }

    @Override
    public Collection<ProcessDefinition> getProcessDefinitions(SnmpManager manager, Location location) {
        boolean enabled = manager.getFeatureManager().isFeatureEnabled(FEATURE, location);
        return (enabled ? Collections.singleton(new ProcessDefinition("sipxacd-stats")) : null);
    }

    @Override
    public void getBundleFeatures(Bundle b) {
    }
}
