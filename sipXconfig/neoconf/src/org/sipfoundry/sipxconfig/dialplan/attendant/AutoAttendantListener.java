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
package org.sipfoundry.sipxconfig.dialplan.attendant;

import java.util.List;

import org.sipfoundry.sipxconfig.cfgmgt.ConfigCommands;
import org.sipfoundry.sipxconfig.common.event.DaoEventListener;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.dialplan.AutoAttendant;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.springframework.beans.factory.annotation.Required;


public class AutoAttendantListener implements DaoEventListener {
    private ConfigCommands m_configCommands;
    private FeatureManager m_featureManager;

    public void onDelete(Object entity) {
        syncPrompts(entity);
    }

    public void onSave(Object entity) {
        syncPrompts(entity);
    }

    private void syncPrompts(Object entity) {
        if (entity instanceof AutoAttendant) {
            List<Location> locations = m_featureManager.getLocationsForEnabledFeature(AutoAttendants.FEATURE);
            for (Location location : locations) {
                if (!location.isPrimary()) {
                    m_configCommands.syncAutoAttendantPrompts(location);
                }
            }
        }
    }

    @Required
    public void setConfigCommands(ConfigCommands configCommands) {
        m_configCommands = configCommands;
    }

    @Required
    public void setFeatureManager(FeatureManager featureManager) {
        m_featureManager = featureManager;
    }

}
