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
package org.sipfoundry.sipxconfig.sbc;

import java.util.List;

import org.sipfoundry.sipxconfig.bridge.BridgeSbc;
import org.sipfoundry.sipxconfig.common.event.DaoEventListener;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.nattraversal.NatTraversal;

public class SbcOnEditListener implements DaoEventListener {
    private SbcDeviceManager m_sbcDeviceManager;
    private SbcManager m_sbcManager;
    private FeatureManager m_featureManager;

    @Override
    public void onDelete(Object entity) {
        if (entity instanceof SbcDevice) {
            SbcDevice device = (SbcDevice) entity;
            List<Sbc> sbcs = m_sbcDeviceManager.getSbcsForSbcDeviceId(device.getId());
            for (Sbc sbc : sbcs) {
                if (sbc.onDeleteSbcDevice()) {
                    m_sbcManager.deleteSbc(sbc);
                } else {
                    m_sbcManager.saveSbc(sbc);
                }
            }
        } else if (entity instanceof Location) {
            BridgeSbc sbc = m_sbcDeviceManager.getBridgeSbc((Location) entity);
            if (sbc != null) {
                m_sbcDeviceManager.deleteSbcDevice(sbc.getId());
            }
        }
    }

    @Override
    public void onSave(Object entity) {
        if (entity instanceof Sbc) {
            m_featureManager.enableGlobalFeature(NatTraversal.FEATURE, true);
        }
    }

    public void setSbcDeviceManager(SbcDeviceManager sbcDeviceManager) {
        m_sbcDeviceManager = sbcDeviceManager;
    }

    public void setSbcManager(SbcManager sbcManager) {
        m_sbcManager = sbcManager;
    }

    public void setFeatureManager(FeatureManager featureManager) {
        m_featureManager = featureManager;
    }
}
