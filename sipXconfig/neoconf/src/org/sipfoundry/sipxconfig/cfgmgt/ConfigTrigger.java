/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.cfgmgt;

import java.util.Collection;

import org.sipfoundry.sipxconfig.common.event.DaoEventListener;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.feature.Feature;
import org.sipfoundry.sipxconfig.feature.FeatureListener;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.feature.GlobalFeature;
import org.sipfoundry.sipxconfig.feature.LocationFeature;

public class ConfigTrigger implements DaoEventListener, FeatureListener {
    private ConfigManager m_configManager;

    @Override
    public void onDelete(Object entity) {
        if (entity instanceof DeployConfigOnEdit) {
            onChange((DeployConfigOnEdit) entity);
        }
    }

    @Override
    public void onSave(Object entity) {
        if (entity instanceof DeployConfigOnEdit) {
            onChange((DeployConfigOnEdit) entity);
        }
    }

    private void onChange(DeployConfigOnEdit entity) {
        Collection<Feature> affected = entity.getAffectedFeaturesOnChange();
        if (affected != null && !affected.isEmpty()) {
            for (Feature f : affected) {
                m_configManager.configureEverywhere(f);
            }
        }
    }

    public void setConfigManager(ConfigManager configManager) {
        m_configManager = configManager;
    }

    @Override
    public void enableLocationFeature(FeatureManager manager, FeatureEvent event, LocationFeature feature,
            Location location) {
        if (event == FeatureEvent.POST_DISABLE || event == FeatureEvent.POST_ENABLE) {
            // even though location is given, we configure everywhere because it's up to the
            // ConfigProvider to decide
            m_configManager.configureEverywhere(feature);
        }
    }

    @Override
    public void enableGlobalFeature(FeatureManager manager, FeatureEvent event, GlobalFeature feature) {
        if (event == FeatureEvent.POST_DISABLE || event == FeatureEvent.POST_ENABLE) {
            m_configManager.configureEverywhere(feature);
        }
    }
}
