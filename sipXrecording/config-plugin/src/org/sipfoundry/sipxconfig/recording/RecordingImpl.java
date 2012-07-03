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
package org.sipfoundry.sipxconfig.recording;

import java.util.Collection;
import java.util.Collections;

import org.sipfoundry.sipxconfig.apache.ApacheManager;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.conference.ConferenceBridgeContext;
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

public class RecordingImpl implements FeatureProvider, Recording, ProcessProvider {
    private BeanWithSettingsDao<RecordingSettings> m_settingsDao;

    @Override
    public Collection<GlobalFeature> getAvailableGlobalFeatures(FeatureManager featureManager) {
        return null;
    }

    @Override
    public Collection<LocationFeature> getAvailableLocationFeatures(FeatureManager featureManager, Location l) {
        return Collections.singleton(FEATURE);
    }

    public RecordingSettings getSettings() {
        return m_settingsDao.findOrCreateOne();
    }

    public void setSettingsDao(BeanWithSettingsDao<RecordingSettings> settingsDao) {
        m_settingsDao = settingsDao;
    }

    public void saveSettings(RecordingSettings settings) {
        m_settingsDao.upsert(settings);
    }
    @Override
    public void getBundleFeatures(FeatureManager featureManager, Bundle b) {
        if (b == Bundle.CORE_TELEPHONY) {
            b.addFeature(FEATURE);
        }
    }

    @Override
    public Collection<ProcessDefinition> getProcessDefinitions(SnmpManager manager, Location location) {
        boolean conf = manager.getFeatureManager().isFeatureEnabled(ConferenceBridgeContext.FEATURE, location);
        boolean rec = manager.getFeatureManager().isFeatureEnabled(Recording.FEATURE, location);
        if (!rec || !conf) {
            return null;
        }
        return Collections.singleton(ProcessDefinition.sipxDefault("sipxrecording",
                ".*\\s-Dprocname=sipxrecording\\s.*"));
    }

    @Override
    public int getJettyPort() {
        return getSettings().getJettyPort();
    }

    @Override
    public void featureChangePrecommit(FeatureManager manager, FeatureChangeValidator validator) {
        validator.requiredOnSameHost(ConferenceBridgeContext.FEATURE, ApacheManager.FEATURE);
        validator.requiredOnSameHost(FEATURE, ConferenceBridgeContext.FEATURE);
    }

    @Override
    public void featureChangePostcommit(FeatureManager manager, FeatureChangeRequest request) {
    }
}