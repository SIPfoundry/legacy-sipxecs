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
package org.sipfoundry.sipxconfig.cfgmgt;

import java.util.Collection;
import java.util.Collections;
import java.util.Set;

import org.sipfoundry.sipxconfig.common.event.DaoEventListener;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.feature.Feature;
import org.sipfoundry.sipxconfig.feature.FeatureChangeRequest;
import org.sipfoundry.sipxconfig.feature.FeatureChangeValidator;
import org.sipfoundry.sipxconfig.feature.FeatureListener;
import org.sipfoundry.sipxconfig.feature.FeatureManager;

public class ConfigTrigger implements DaoEventListener, FeatureListener {
    private ConfigManager m_configManager;

    @Override
    public void onDelete(Object entity) {
        onEntityChange(entity);
    }

    @Override
    public void onSave(Object entity) {
        onEntityChange(entity);
    }

    private void onEntityChange(Object entity) {
        if (entity instanceof Location) {
            Collection<Location> reset = Collections.singleton((Location) entity);
            // reset on add or delete incase last delete didn't complete
            m_configManager.resetKeys(reset);
        }
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
    public void featureChangePrecommit(FeatureManager manager, FeatureChangeValidator request) {
    }

    @Override
    public void featureChangePostcommit(FeatureManager manager, FeatureChangeRequest request) {
        // even though location is given, we configure everywhere because it's up to the
        // ConfigProvider to decide
        Set<Feature> changes =  request.getAllNewlyEnabledFeatures();
        changes.addAll(request.getAllNewlyDisabledFeatures());
        m_configManager.configureEverywhere(changes.toArray(new Feature[0]));
    }
}
