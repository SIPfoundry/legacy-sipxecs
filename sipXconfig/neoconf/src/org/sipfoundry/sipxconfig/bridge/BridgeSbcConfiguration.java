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
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.sipfoundry.sipxconfig.alarm.AlarmDefinition;
import org.sipfoundry.sipxconfig.alarm.AlarmProvider;
import org.sipfoundry.sipxconfig.alarm.AlarmServerManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigProvider;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigRequest;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigUtils;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.device.ProfileLocation;
import org.sipfoundry.sipxconfig.feature.Bundle;
import org.sipfoundry.sipxconfig.feature.FeatureChangeRequest;
import org.sipfoundry.sipxconfig.feature.FeatureChangeValidator;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.feature.FeatureProvider;
import org.sipfoundry.sipxconfig.feature.GlobalFeature;
import org.sipfoundry.sipxconfig.feature.LocationFeature;
import org.sipfoundry.sipxconfig.nattraversal.NatTraversal;
import org.sipfoundry.sipxconfig.proxy.ProxyManager;
import org.sipfoundry.sipxconfig.sbc.SbcDeviceManager;
import org.sipfoundry.sipxconfig.snmp.ProcessDefinition;
import org.sipfoundry.sipxconfig.snmp.ProcessProvider;
import org.sipfoundry.sipxconfig.snmp.SnmpManager;
import org.sipfoundry.sipxconfig.tls.TlsPeerManager;

public class BridgeSbcConfiguration implements ConfigProvider, ProcessProvider, FeatureProvider, AlarmProvider {
    // uses of this definition are not related, just defined in one place to avoid checkstyle err
    private static final String SIPXBRIDGE = "sipxbridge";
    private SbcDeviceManager m_sbcDeviceManager;

    @Override
    public void replicate(ConfigManager manager, ConfigRequest request) throws IOException {
        if (!request.applies(ProxyManager.FEATURE, BridgeSbcContext.FEATURE, TlsPeerManager.FEATURE)) {
            return;
        }

        Set<Location> locations = request.locations(manager);
        List<BridgeSbc> bridges = m_sbcDeviceManager.getBridgeSbcs();
        Map<Integer, BridgeSbc> bridgesMap = new HashMap<Integer, BridgeSbc>();
        for (BridgeSbc bridge : bridges) {
            bridgesMap.put(bridge.getLocation().getId(), bridge);
        }
        for (Location location : locations) {
            BridgeSbc bridge = bridgesMap.get(location.getId());
            boolean bridgeHere = bridge != null ? true : false;
            File dir = manager.getLocationDataDirectory(location);
            ConfigUtils.enableCfengineClass(dir, "sipxbridge.cfdat", bridgeHere, SIPXBRIDGE);
            if (bridgeHere) {
                // strange object for profile location to be compatible with device module
                ProfileLocation profileLocation = bridge.getProfileLocation();
                bridge.generateFiles(profileLocation);
            }
        }
    }

    public void setSbcDeviceManager(SbcDeviceManager sbcDeviceManager) {
        m_sbcDeviceManager = sbcDeviceManager;
    }

    @Override
    public Collection<ProcessDefinition> getProcessDefinitions(SnmpManager manager, Location location) {
        boolean enabled = manager.getFeatureManager().isFeatureEnabled(BridgeSbcContext.FEATURE, location);
        return (enabled ? Collections
                .singleton(ProcessDefinition.sipxByRegex(SIPXBRIDGE, ".*\\s-Dprocname=sipxbridge\\s.*")) : null);
    }

    @Override
    public void featureChangePrecommit(FeatureManager manager, FeatureChangeValidator validator) {
        validator.requiresGlobalFeature(BridgeSbcContext.FEATURE, NatTraversal.FEATURE);
        validator.requiresAtLeastOne(BridgeSbcContext.FEATURE, ProxyManager.FEATURE);
    }

    @Override
    public void featureChangePostcommit(FeatureManager manager, FeatureChangeRequest request) {
        if (request.hasChanged(BridgeSbcContext.FEATURE)) {
            for (Location l : request.getLocationsForEnabledFeature(BridgeSbcContext.FEATURE)) {
                BridgeSbc bridgeSbc = m_sbcDeviceManager.getBridgeSbc(l);
                if (bridgeSbc == null) {
                    m_sbcDeviceManager.newBridgeSbc(l);
                }
            }
            for (Location l : request.getLocationsForDisabledFeature(BridgeSbcContext.FEATURE)) {
                BridgeSbc bridgeSbc = m_sbcDeviceManager.getBridgeSbc(l);
                if (bridgeSbc != null) {
                    m_sbcDeviceManager.deleteSbcDevice(bridgeSbc.getId());
                }
            }
        }
    }

    @Override
    public Collection<GlobalFeature> getAvailableGlobalFeatures(FeatureManager featureManager) {
        // TODO Auto-generated method stub
        return null;
    }

    @Override
    public Collection<LocationFeature> getAvailableLocationFeatures(FeatureManager featureManager, Location l) {
        // TODO Auto-generated method stub
        return null;
    }

    @Override
    public void getBundleFeatures(FeatureManager featureManager, Bundle b) {
        if (b == Bundle.CORE_TELEPHONY) {
            b.addFeature(BridgeSbcContext.FEATURE);
        }
    }

    @Override
    public Collection<AlarmDefinition> getAvailableAlarms(AlarmServerManager manager) {
        if (!manager.getFeatureManager().isFeatureEnabled(BridgeSbcContext.FEATURE)) {
            return null;
        }
        String[] ids = new String[] {
            "BRIDGE_STUN_FAILURE", "BRIDGE_STUN_RECOVERY", "BRIDGE_STUN_PUBLIC_ADDRESS_CHANGED",
            "BRIDGE_ACCOUNT_NOT_FOUND", "BRIDGE_ACCOUNT_CONFIGURATION_ERROR", "BRIDGE_OPERATION_TIMED_OUT",
            "BRIDGE_ITSP_SERVER_FAILURE", "BRIDGE_AUTHENTICATION_FAILED", "BRIDGE_ITSP_ACCOUNT_CONFIGURATION_ERROR",
            "BRIDGE_TLS_CERTIFICATE_MISMATCH", "BRIDGE_ACCOUNT_OK"
        };

        return AlarmDefinition.asArray(ids);
    }
}
