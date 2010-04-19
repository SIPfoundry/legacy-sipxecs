/*
 *
 *
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxconfig.gateway.audiocodes;

import org.sipfoundry.sipxconfig.gateway.Gateway;
import org.sipfoundry.sipxconfig.setting.Setting;

/**
 * Object representing Ip2Tel route on an AudioCodes PSTN gateway
 */
public class Ip2TelRoute extends RouteTable {
    private boolean m_initialized;

    @Override
    protected Setting loadSettings() {
        return null;
    }

    @Override
    public synchronized void initialize() {
        if (m_initialized) {
            return;
        }
        Gateway gateway = getGateway();
        if (gateway != null) {
            if (gateway instanceof AudioCodesGateway) {
                ((AudioCodesGateway) gateway).initializeIp2TelRoute(this);
                m_initialized = true;
            }
        }
    }

}
