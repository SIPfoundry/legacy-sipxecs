/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
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
