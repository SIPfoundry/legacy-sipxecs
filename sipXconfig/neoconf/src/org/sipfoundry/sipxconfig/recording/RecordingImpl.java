/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.recording;

import java.util.Collection;
import java.util.Collections;

import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.conference.ConferenceBridgeContext;
import org.sipfoundry.sipxconfig.feature.Bundle;
import org.sipfoundry.sipxconfig.feature.FeatureProvider;
import org.sipfoundry.sipxconfig.feature.GlobalFeature;
import org.sipfoundry.sipxconfig.feature.LocationFeature;
import org.sipfoundry.sipxconfig.setting.BeanWithSettingsDao;
import org.sipfoundry.sipxconfig.snmp.ProcessDefinition;
import org.sipfoundry.sipxconfig.snmp.ProcessProvider;
import org.sipfoundry.sipxconfig.snmp.SnmpManager;

public class RecordingImpl implements FeatureProvider, Recording, ProcessProvider {
    private BeanWithSettingsDao<RecordingSettings> m_settingsDao;

    public RecordingSettings getSettings() {
        return m_settingsDao.findOrCreateOne();
    }

    public void saveSettings(RecordingSettings settings) {
        m_settingsDao.upsert(settings);
    }

    @Override
    public Collection<GlobalFeature> getAvailableGlobalFeatures() {
        return Collections.singleton(FEATURE);
    }

    @Override
    public Collection<LocationFeature> getAvailableLocationFeatures(Location l) {
        return null;
    }

    public void setSettingsDao(BeanWithSettingsDao<RecordingSettings> settingsDao) {
        m_settingsDao = settingsDao;
    }

    @Override
    public Collection<ProcessDefinition> getProcessDefinitions(SnmpManager manager, Location location) {
        boolean conf = manager.getFeatureManager().isFeatureEnabled(ConferenceBridgeContext.FEATURE, location);
        boolean rec = manager.getFeatureManager().isFeatureEnabled(Recording.FEATURE);
        return (conf && rec ? Collections.singleton(new ProcessDefinition("sipxrecording",
            ".*\\s-Dprocname=sipxrecording\\s.*")) : null);
    }

    @Override
    public void getBundleFeatures(Bundle b) {
        if (b.isBasic()) {
            b.addFeature(FEATURE);
        }
    }
}
