/**
 *
 *
 * Copyright (c) 2013 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.provision;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.setup.SetupListener;
import org.sipfoundry.sipxconfig.setup.SetupManager;

/*
 * This task will disable the auto provision feature on any secondary nodes.
 * According to XX-10755 auto provision feature is a "primary only" feature.
 */
public class PrimaryOnlyMigrationTask implements SetupListener {
    private static final Log LOG = LogFactory.getLog(PrimaryOnlyMigrationTask.class);
    private static final String PROVISION_DISABLE_TASK = "provision_disable_on_secondary";

    private FeatureManager m_featureManager;

    @Override
    public boolean setup(SetupManager manager) {
        if (manager.isFalse(PROVISION_DISABLE_TASK)) {
            for (Location l : m_featureManager.getLocationsForEnabledFeature(Provision.FEATURE)) {
                if (!l.isPrimary()) {
                    LOG.info(String.format("Disabling provision on %s", l.getFqdn()));
                    m_featureManager.enableLocationFeature(Provision.FEATURE, l, false);
                }
            }
            manager.setTrue(PROVISION_DISABLE_TASK);
        }
        return true;
    }

    public void setFeatureManager(FeatureManager featureManager) {
        m_featureManager = featureManager;
    }

}
