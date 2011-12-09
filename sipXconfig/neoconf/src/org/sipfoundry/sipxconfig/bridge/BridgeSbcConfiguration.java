/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.bridge;

import java.io.IOException;
import java.util.List;

import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigProvider;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigRequest;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.device.ProfileLocation;
import org.sipfoundry.sipxconfig.feature.FeatureListener;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.feature.GlobalFeature;
import org.sipfoundry.sipxconfig.feature.LocationFeature;
import org.sipfoundry.sipxconfig.proxy.ProxyManager;
import org.sipfoundry.sipxconfig.sbc.SbcDeviceManager;
import org.sipfoundry.sipxconfig.tls.TlsPeerManager;

public class BridgeSbcConfiguration implements ConfigProvider, FeatureListener {
    private SbcDeviceManager m_sbcDeviceManager;
    private FeatureManager m_featureManager;

    @Override
    public void replicate(ConfigManager manager, ConfigRequest request) throws IOException {
        if (!request.applies(BridgeSbcContext.FEATURE, TlsPeerManager.FEATURE)) {
            return;
        }

        List<BridgeSbc> bridges = m_sbcDeviceManager.getBridgeSbcs();
        List<Location> locations = manager.getFeatureManager().getLocationsForEnabledFeature(
                BridgeSbcContext.FEATURE);
        for (BridgeSbc bridge : bridges) {
            Location location = bridge.getLocation();
            if (locations.contains(location)) {
                ProfileLocation profileLocation = bridge.getProfileLocation();
                bridge.generateFiles(profileLocation);
            }
        }
    }

    @Override
    public void enableLocationFeature(FeatureEvent event, LocationFeature feature, Location location) {
        if (!feature.equals(ProxyManager.FEATURE)) {
            return;
        }

        if (!m_featureManager.isFeatureEnabled(BridgeSbcContext.FEATURE)) {
            return;
        }

        BridgeSbc bridgeSbc = m_sbcDeviceManager.getBridgeSbc(location);
        if (event == FeatureEvent.PRE_ENABLE && bridgeSbc == null) {
            m_sbcDeviceManager.newBridgeSbc(location);
        }
        if (event == FeatureEvent.POST_DISABLE && bridgeSbc != null) {
            m_sbcDeviceManager.deleteSbcDevice(bridgeSbc.getId());
        }
    }

    @Override
    public void enableGlobalFeature(FeatureEvent event, GlobalFeature feature) {
    }
}
