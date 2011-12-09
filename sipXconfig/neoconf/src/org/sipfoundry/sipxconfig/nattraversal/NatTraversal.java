/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.nattraversal;

import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.feature.FeatureListener;
import org.sipfoundry.sipxconfig.feature.GlobalFeature;
import org.sipfoundry.sipxconfig.feature.LocationFeature;
import org.sipfoundry.sipxconfig.setting.BeanWithSettingsDao;

public class NatTraversal implements FeatureListener {
    public static final GlobalFeature FEATURE = new GlobalFeature("natTraversal");
    private BeanWithSettingsDao<NatSettings> m_settingsDao;

    public NatSettings getSettings() {
        return m_settingsDao.findOne();
    }

    public void saveSettings(NatSettings settings) {
        m_settingsDao.upsert(settings);
    }

    @Override
    public void enableLocationFeature(FeatureEvent event, LocationFeature feature, Location location) {
    }

    @Override
    public void enableGlobalFeature(FeatureEvent event, GlobalFeature feature) {
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
}
