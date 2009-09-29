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

import org.sipfoundry.sipxconfig.device.DeviceSource;

public class GatewaySource implements DeviceSource<Gateway> {

    private GatewayContext m_gatewayContext;

    public void setGatewayContext(GatewayContext gatewayContext) {
        m_gatewayContext = gatewayContext;
    }

    public Gateway loadDevice(Integer id) {
        return m_gatewayContext.getGateway(id);
    }
}
