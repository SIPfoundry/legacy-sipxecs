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
package org.sipfoundry.sipxconfig.bridge;

import java.io.File;
import java.io.IOException;
import java.util.Collection;
import java.util.Collections;
import java.util.List;
import java.util.Set;

import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigProvider;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigRequest;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigUtils;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.device.ProfileLocation;
import org.sipfoundry.sipxconfig.feature.FeatureListener;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.feature.GlobalFeature;
import org.sipfoundry.sipxconfig.feature.LocationFeature;
import org.sipfoundry.sipxconfig.proxy.ProxyManager;
import org.sipfoundry.sipxconfig.sbc.SbcDeviceManager;
import org.sipfoundry.sipxconfig.snmp.ProcessDefinition;
import org.sipfoundry.sipxconfig.snmp.ProcessProvider;
import org.sipfoundry.sipxconfig.snmp.SnmpManager;
import org.sipfoundry.sipxconfig.tls.TlsPeerManager;

public class BridgeSbcConfiguration implements ConfigProvider, FeatureListener, ProcessProvider {
    // uses of this definition are not related, just defined in one place to avoid checkstyle err
    private static final String SIPXBRIDGE = "sipxbridge";
    private SbcDeviceManager m_sbcDeviceManager;

    @Override
    public void replicate(ConfigManager manager, ConfigRequest request) throws IOException {
        if (!request.applies(ProxyManager.FEATURE, BridgeSbcContext.FEATURE, TlsPeerManager.FEATURE)) {
            return;
        }

        Set<Location> locations = request.locations(manager);
        FeatureManager fm = manager.getFeatureManager();
        List<BridgeSbc> bridges = m_sbcDeviceManager.getBridgeSbcs();
        for (Location l : locations) {
            for (BridgeSbc bridge : bridges) {
                Location location = bridge.getLocation();
                boolean bridgeHere = l.getId().equals(location.getId());
                File dir = manager.getLocationDataDirectory(location);
                ConfigUtils.enableCfengineClass(dir, "sipxbridge.cfdat", bridgeHere, SIPXBRIDGE);
                boolean proxyHere = manager.getFeatureManager().isFeatureEnabled(ProxyManager.FEATURE, location);
                // Proxy source reads sipxbrige.xml to find how to connect to bridge
                if (bridgeHere || proxyHere) {
                    // strange object for profile location to be compatible with device module
                    ProfileLocation profileLocation = bridge.getProfileLocation();
                    bridge.generateFiles(profileLocation);
                }
            }
        }
    }

    @Override
    public void enableLocationFeature(FeatureManager manager, FeatureEvent event, LocationFeature feature,
            Location location) {

        if (feature.equals(ProxyManager.FEATURE) && event == FeatureEvent.PRE_ENABLE) {
            // HACK: Proxy requires one or more bridges to be running on your system
            // this should in turn call this function again with BridgeFeature on
            if (!manager.isFeatureEnabled(BridgeSbcContext.FEATURE)) {
                manager.enableLocationFeature(BridgeSbcContext.FEATURE, location, true);
            }
        }

        if (!feature.equals(BridgeSbcContext.FEATURE)) {
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
    public void enableGlobalFeature(FeatureManager manager, FeatureEvent event, GlobalFeature feature) {
    }

    public void setSbcDeviceManager(SbcDeviceManager sbcDeviceManager) {
        m_sbcDeviceManager = sbcDeviceManager;
    }

    @Override
    public Collection<ProcessDefinition> getProcessDefinitions(SnmpManager manager, Location location) {
        boolean enabled = manager.getFeatureManager().isFeatureEnabled(BridgeSbcContext.FEATURE, location);
        return (enabled ? Collections.singleton(new ProcessDefinition(SIPXBRIDGE,
                ".*\\s-Dprocname=sipxbridge\\s.*")) : null);
    }
}
