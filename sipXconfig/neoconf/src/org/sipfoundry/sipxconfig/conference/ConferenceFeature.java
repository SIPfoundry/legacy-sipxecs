/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.conference;

import java.util.Collection;
import java.util.Collections;

import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.feature.Bundle;
import org.sipfoundry.sipxconfig.feature.FeatureListener;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.feature.FeatureProvider;
import org.sipfoundry.sipxconfig.feature.GlobalFeature;
import org.sipfoundry.sipxconfig.feature.LocationFeature;

public class ConferenceFeature implements FeatureListener, FeatureProvider {
    private ConferenceBridgeContext m_conferenceBridgeContext;

    @Override
    public Collection<GlobalFeature> getAvailableGlobalFeatures() {
        return null;
    }

    @Override
    public Collection<LocationFeature> getAvailableLocationFeatures(Location l) {
        return Collections.singleton(ConferenceBridgeContext.FEATURE);
    }

    @Override
    public void enableLocationFeature(FeatureManager manager, FeatureEvent event, LocationFeature feature,
            Location location) {
        if (!feature.equals(ConferenceBridgeContext.FEATURE)) {
            return;
        }

        Bridge bridge = m_conferenceBridgeContext.getBridgeByServer(location.getFqdn());
        if (bridge == null && event == FeatureEvent.PRE_ENABLE) {
            bridge = m_conferenceBridgeContext.newBridge();
            bridge.setLocation(location);
            m_conferenceBridgeContext.store(bridge);
        }
        if (bridge != null && event == FeatureEvent.POST_ENABLE) {
            m_conferenceBridgeContext.deploy(bridge);
        }
    }

    @Override
    public void enableGlobalFeature(FeatureManager manager, FeatureEvent event, GlobalFeature feature) {
    }

    public void setConferenceBridgeContext(ConferenceBridgeContext conferenceBridgeContext) {
        m_conferenceBridgeContext = conferenceBridgeContext;
    }

    @Override
    public void getBundleFeatures(Bundle b) {
        if (b.isBasic()) {
            b.addFeature(ConferenceBridgeContext.FEATURE);
        }
    }
}
