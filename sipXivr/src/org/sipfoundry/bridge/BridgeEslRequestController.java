/*
 *
 *
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
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
        m_sipReqUri = getSipReqUri();
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
