/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.gateway;

import org.sipfoundry.sipxconfig.device.AbstractProfileManager;
import org.sipfoundry.sipxconfig.device.Device;

public class GatewayProfileManagerImpl extends AbstractProfileManager {

    private GatewayContext m_gatewayContext;

    public void setGatewayContext(GatewayContext gatewayContext) {
        m_gatewayContext = gatewayContext;
    }

    @Override
    protected Device loadDevice(Integer id) {
        return m_gatewayContext.getGateway(id);
    }

    @Override
    protected void restartDevice(Integer id) {
        // TODO: need a way to restart gateways
    }
}
