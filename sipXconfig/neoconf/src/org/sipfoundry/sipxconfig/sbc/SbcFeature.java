/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.sbc;

import java.util.Collection;
import java.util.Collections;

import org.sipfoundry.sipxconfig.bridge.BridgeSbc;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.feature.FeatureListener;
import org.sipfoundry.sipxconfig.feature.FeatureProvider;
import org.sipfoundry.sipxconfig.feature.GlobalFeature;
import org.sipfoundry.sipxconfig.feature.LocationFeature;

public class SbcFeature implements FeatureProvider, FeatureListener {
    public static final LocationFeature SBC = new LocationFeature("borderController");
    private SbcDeviceManager m_sbcDeviceManager;

    @Override
    public void enableLocationFeature(FeatureEvent event, LocationFeature feature, Location location) {
        if (feature.equals(SBC)) {
            BridgeSbc bridgeSbc = m_sbcDeviceManager.getBridgeSbc(location);
            if (event == FeatureEvent.PRE_ENABLE && bridgeSbc == null) {
                m_sbcDeviceManager.newSbc(location);
            }
            if (event == FeatureEvent.POST_DISABLE && bridgeSbc != null) {
                m_sbcDeviceManager.deleteSbcDevice(bridgeSbc.getId());
            }
        }
    }

    @Override
    public void enableGlobalFeature(FeatureEvent event, GlobalFeature feature) {
    }

    public void setSbcDeviceManager(SbcDeviceManager sbcDeviceManager) {
        m_sbcDeviceManager = sbcDeviceManager;
    }

    @Override
    public Collection<GlobalFeature> getAvailableGlobalFeatures() {
        return null;
    }

    @Override
    public Collection<LocationFeature> getAvailableLocationFeatures(Location l) {
        return Collections.singleton(SBC);
    }

}
