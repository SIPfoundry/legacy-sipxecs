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
package org.sipfoundry.sipxconfig.gateway;

import java.util.List;

import org.sipfoundry.sipxconfig.common.event.DaoEventListener;
import org.sipfoundry.sipxconfig.sbc.SbcDevice;

public class GatewayOnEdit implements DaoEventListener {
    private GatewayContext m_gatewayContext;

    @Override
    public void onDelete(Object entity) {
        if (!(entity instanceof SbcDevice)) {
            return;
        }
        SbcDevice sbcDevice = (SbcDevice) entity;
        List< ? extends SipTrunk> sipTrunks = m_gatewayContext.getGatewayByType(SipTrunk.class);
        for (SipTrunk sipTrunk : sipTrunks) {
            if (sbcDevice.equals(sipTrunk.getSbcDevice())) {
                sipTrunk.setSbcDevice(null);
                m_gatewayContext.saveGateway(sipTrunk);
            }
        }
    }

    @Override
    public void onSave(Object entity) {
    }

    public void setGatewayContext(GatewayContext gatewayContext) {
        m_gatewayContext = gatewayContext;
    }

}
