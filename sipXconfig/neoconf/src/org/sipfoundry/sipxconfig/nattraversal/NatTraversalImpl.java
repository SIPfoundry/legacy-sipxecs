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


import java.util.Collection;
import java.util.Collections;

import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.feature.Bundle;
import org.sipfoundry.sipxconfig.feature.FeatureListener;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.feature.FeatureProvider;
import org.sipfoundry.sipxconfig.feature.GlobalFeature;
import org.sipfoundry.sipxconfig.feature.LocationFeature;
import org.sipfoundry.sipxconfig.proxy.ProxyManager;
import org.sipfoundry.sipxconfig.setting.BeanWithSettingsDao;
import org.sipfoundry.sipxconfig.snmp.ProcessDefinition;
import org.sipfoundry.sipxconfig.snmp.ProcessProvider;
import org.sipfoundry.sipxconfig.snmp.SnmpManager;

public class NatTraversalImpl implements FeatureListener, NatTraversal, FeatureProvider, ProcessProvider {
    private BeanWithSettingsDao<NatSettings> m_settingsDao;

    public NatSettings getSettings() {
        return m_settingsDao.findOrCreateOne();
    }

    public void saveSettings(NatSettings settings) {
        m_settingsDao.upsert(settings);
    }

    @Override
    public void enableLocationFeature(FeatureManager manager, FeatureEvent event, LocationFeature feature,
            Location location) {
    }

    @Override
    public void enableGlobalFeature(FeatureManager manager, FeatureEvent event, GlobalFeature feature) {
        if (feature.equals(FEATURE)) {
            if (event == FeatureEvent.PRE_ENABLE) {
                NatSettings settings = getSettings();
                if (!settings.isBehindNat()) {
                    settings.setBehindNat(true);
                    saveSettings(settings);
                }
            } else if (event == FeatureEvent.PRE_DISABLE) {
                NatSettings settings = getSettings();
                if (settings.isBehindNat()) {
                    settings.setBehindNat(false);
                    saveSettings(settings);
                }
            }
        }
    }

    public void setSettingsDao(BeanWithSettingsDao<NatSettings> settingsDao) {
        m_settingsDao = settingsDao;
    }

    @Override
    public Collection<GlobalFeature> getAvailableGlobalFeatures() {
        return Collections.singleton(FEATURE);
    }

    @Override
    public Collection<LocationFeature> getAvailableLocationFeatures(Location l) {
        return null;
    }

    @Override
    public Collection<ProcessDefinition> getProcessDefinitions(SnmpManager manager, Location location) {
        boolean relayEnabled = manager.getFeatureManager().isFeatureEnabled(FEATURE);
        boolean proxyEnabled = manager.getFeatureManager().isFeatureEnabled(ProxyManager.FEATURE, location);
        return (relayEnabled && proxyEnabled ? Collections.singleton(new ProcessDefinition("sipxrelay",
            ".*\\s-Dprocname=sipxrelay\\s.*")) : null);
    }

    @Override
    public void getBundleFeatures(Bundle b) {
        if (b.isRouter()) {
            // NAT traveral is debatable but proxy requires it ATM AFAIU
            b.addFeature(FEATURE);
        }
    }
}
