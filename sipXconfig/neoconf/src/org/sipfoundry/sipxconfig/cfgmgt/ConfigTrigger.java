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
import org.sipfoundry.sipxconfig.feature.Feature;

public class ConfigTrigger implements DaoEventListener {
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
}
