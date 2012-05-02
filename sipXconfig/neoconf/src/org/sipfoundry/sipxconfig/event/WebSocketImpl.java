/**
 *
 *
 * Copyright (c) 2010 / 2012 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.event;

import java.util.Collection;
import java.util.Collections;

import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.feature.Bundle;
import org.sipfoundry.sipxconfig.feature.FeatureChangeRequest;
import org.sipfoundry.sipxconfig.feature.FeatureChangeValidator;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.feature.FeatureProvider;
import org.sipfoundry.sipxconfig.feature.GlobalFeature;
import org.sipfoundry.sipxconfig.feature.LocationFeature;
import org.sipfoundry.sipxconfig.setting.BeanWithSettingsDao;
import org.sipfoundry.sipxconfig.snmp.ProcessDefinition;
import org.sipfoundry.sipxconfig.snmp.ProcessProvider;
import org.sipfoundry.sipxconfig.snmp.SnmpManager;

public class WebSocketImpl implements FeatureProvider, WebSocket, ProcessProvider {
    private BeanWithSettingsDao<WebSocketSettings> m_settingsDao;

    @Override
    public Collection<GlobalFeature> getAvailableGlobalFeatures(FeatureManager featureManager) {
        return null;
    }

    @Override
    public Collection<LocationFeature> getAvailableLocationFeatures(FeatureManager featureManager, Location l) {
        return Collections.singleton(FEATURE);
    }

    public WebSocketSettings getSettings() {
        return m_settingsDao.findOrCreateOne();
    }

    public void setSettingsDao(BeanWithSettingsDao<WebSocketSettings> settingsDao) {
        m_settingsDao = settingsDao;
    }

    public void saveSettings(WebSocketSettings settings) {
        m_settingsDao.upsert(settings);
    }
    @Override
    public void getBundleFeatures(FeatureManager featureManager, Bundle b) {
        if (b == Bundle.CORE) {
            b.addFeature(FEATURE);
        }
    }

    @Override
    public Collection<ProcessDefinition> getProcessDefinitions(SnmpManager manager, Location location) {
        boolean enabled = manager.getFeatureManager().isFeatureEnabled(FEATURE, location);
        return (enabled ? Collections.singleton(
                new ProcessDefinition("sipxevent", ".*\\s-Dprocname=sipxevent\\s.*")) : null);
    }

    @Override
    public void featureChangePrecommit(FeatureManager manager, FeatureChangeValidator validator) {
        // components this required are not available yet -- Douglas
    }

    @Override
    public void featureChangePostcommit(FeatureManager manager, FeatureChangeRequest request) {
    }
}
