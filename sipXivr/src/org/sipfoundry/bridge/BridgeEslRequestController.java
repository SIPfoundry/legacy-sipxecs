/**
 *
 *
 * Copyright (c) 2011 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.bridge;

import java.util.Hashtable;

import org.sipfoundry.commons.freeswitch.BridgeCommand;
import org.sipfoundry.sipxivr.eslrequest.AbstractEslRequestController;

public class BridgeEslRequestController extends AbstractEslRequestController {

    private String m_sipReqUri;

    @Override
    public void extractParameters(Hashtable<String, String> parameters) {
        String uuid = getCallerUniqueId();
        String sipReqParams = getSipReqParams();
        // bridge always to same node, that is local node
        m_sipReqUri = "IVR@" + getFreeswitchIpAndPort();
        if (uuid == null || getSipxchangeDomainName() == null || sipReqParams == null) {
            hangup();
            return;
        }
        sipReqParams += ";uuid=";
        sipReqParams += uuid;
        m_sipReqUri += ";";
        m_sipReqUri += sipReqParams;
    }

    @Override
    public void loadConfig() {
    }

    public void bridgeCall() {
        BridgeCommand bridge = new BridgeCommand(getFsEventSocket(), getFsEventSocket().getVariable("Unique-ID"),
                m_sipReqUri, getSipxchangeDomainName());
        bridge.start();
    }

}
