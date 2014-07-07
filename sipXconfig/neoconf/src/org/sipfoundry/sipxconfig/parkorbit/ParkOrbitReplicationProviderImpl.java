/**
 *
 *
 * Copyright (c) 2014 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.parkorbit;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import org.sipfoundry.sipxconfig.common.Replicable;
import org.sipfoundry.sipxconfig.feature.FeatureManager;

public class ParkOrbitReplicationProviderImpl implements ParkOrbitReplicationProvider {
    private ParkOrbitContext m_parkContext;
    private FeatureManager m_featureManager;

    @Override
    public List<Replicable> getReplicables() {
        if (m_featureManager.isFeatureEnabled(ParkOrbitContext.FEATURE)) {
            List<Replicable> replicables = new ArrayList<Replicable>();
            replicables.addAll(m_parkContext.getParkOrbits());
            return replicables;
        }
        return Collections.EMPTY_LIST;
    }

    public void setParkOrbitContext(ParkOrbitContext parkContext) {
        m_parkContext = parkContext;
    }

    public void setFeatureManager(FeatureManager featureManager) {
        m_featureManager = featureManager;
    }
}
