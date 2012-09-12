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
package org.sipfoundry.sipxconfig.conference;

import java.util.Collection;
import java.util.Collections;

import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.feature.Bundle;
import org.sipfoundry.sipxconfig.feature.FeatureChangeRequest;
import org.sipfoundry.sipxconfig.feature.FeatureChangeValidator;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.feature.FeatureProvider;
import org.sipfoundry.sipxconfig.feature.GlobalFeature;
import org.sipfoundry.sipxconfig.feature.LocationFeature;
import org.sipfoundry.sipxconfig.freeswitch.FreeswitchFeature;

public class ConferenceFeature implements FeatureProvider {
    private ConferenceBridgeContext m_conferenceBridgeContext;

    @Override
    public Collection<GlobalFeature> getAvailableGlobalFeatures(FeatureManager featureManager) {
        return null;
    }

    @Override
    public Collection<LocationFeature> getAvailableLocationFeatures(FeatureManager featureManager, Location l) {
        return Collections.singleton(ConferenceBridgeContext.FEATURE);
    }

    public void setConferenceBridgeContext(ConferenceBridgeContext conferenceBridgeContext) {
        m_conferenceBridgeContext = conferenceBridgeContext;
    }

    @Override
    public void getBundleFeatures(FeatureManager featureManager, Bundle b) {
        if (b == Bundle.CORE_TELEPHONY) {
            b.addFeature(ConferenceBridgeContext.FEATURE);
        }
    }

    @Override
    public void featureChangePrecommit(FeatureManager manager, FeatureChangeValidator validator) {
        validator.requiredOnSameHost(ConferenceBridgeContext.FEATURE, FreeswitchFeature.FEATURE);
    }

    @Override
    public void featureChangePostcommit(FeatureManager manager, FeatureChangeRequest request) {
        Collection<Location> locations = request.getLocationsForEnabledFeature(ConferenceBridgeContext.FEATURE);

        // create new bridges if needed
        if (request.getAllNewlyEnabledFeatures().contains(ConferenceBridgeContext.FEATURE)) {
            for (Location location : locations) {
                Bridge bridge = m_conferenceBridgeContext.getBridgeByServer(location.getFqdn());
                if (bridge == null) {
                    bridge = m_conferenceBridgeContext.newBridge();
                    bridge.setLocation(location);
                    m_conferenceBridgeContext.saveBridge(bridge);
                }
            }
        }

        // remove old bridges
        if (request.getAllNewlyDisabledFeatures().contains(ConferenceBridgeContext.FEATURE)) {
            for (Bridge bridge : m_conferenceBridgeContext.getBridges()) {
                if (!locations.contains(bridge.getLocation())) {
                    m_conferenceBridgeContext.removeBridge(bridge);
                }
            }
        }
    }
}
