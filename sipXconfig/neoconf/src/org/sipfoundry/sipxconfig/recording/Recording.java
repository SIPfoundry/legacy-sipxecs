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
import org.sipfoundry.sipxconfig.feature.FeatureProvider;
import org.sipfoundry.sipxconfig.feature.GlobalFeature;
import org.sipfoundry.sipxconfig.feature.LocationFeature;
import org.sipfoundry.sipxconfig.setting.BeanWithSettingsDao;

public class Recording implements FeatureProvider {
    public static final GlobalFeature FEATURE = new GlobalFeature("bridgeRecording");
    private BeanWithSettingsDao<RecordingSettings> m_settingsDao;

    public RecordingSettings getSettings() {
        return m_settingsDao.findOne();
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
}
