/*
 * Copyright (c) 2010 eZuce, Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 */

package org.sipfoundry.bridge;

import java.io.File;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Hashtable;
import java.util.Locale;
import java.util.ResourceBundle;

import org.apache.log4j.Logger;
import org.sipfoundry.commons.freeswitch.FreeSwitchEventSocketInterface;
import org.sipfoundry.commons.freeswitch.BridgeCommand;
import org.sipfoundry.commons.freeswitch.Set;
import org.sipfoundry.commons.freeswitch.Hangup;
import org.sipfoundry.sipxivr.IvrConfiguration;


public class Bridge {
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxivr");

    // Global store for AutoAttendant resource bundles keyed by locale
    private static final String RESOURCE_NAME="org.sipfoundry.attendant.AutoAttendant";

    private IvrConfiguration m_ivrConfig;
    private FreeSwitchEventSocketInterface m_fses;
    @SuppressWarnings("unused")

    /**
     * Create an Bridge.
     * 
     * @param ivrConfig top level configuration stuff
     * @param fses The FreeSwitchEventSocket with the call already answered
     * @param parameters The parameters from the sip URI
     */
    public Bridge(IvrConfiguration ivrConfig, FreeSwitchEventSocketInterface fses) {
        this.m_ivrConfig = ivrConfig;
        this.m_fses = fses;
    }

    /**
     * Load all the needed configuration.
     * 
     * The attendantBundle with the resources is located based on locale, as is the TextToPrompts
     * class. The attendant configuration files are loaded (if they changed since last time), and
     * the ValidUsers (also if they changed).
     *      * 
     */
    void loadConfig() {}


    /**
     * Run the bridge. Bridge the call back to freeswitch with all the parameters intact
     * 
     * @throws Throwable indicating an error or hangup condition.
     */
    public void run() {
        String uuid = m_fses.getVariable("Caller-Unique-ID");
        String domain = m_ivrConfig.getSipxchangeDomainName();
        String sipReqParams = m_fses.getVariable("variable_sip_req_params");
        String sipReqUri = m_fses.getVariable("variable_sip_req_uri");
        if (uuid == null || domain == null || sipReqParams == null )
        {
            //
            // Disconnect the call and exit
            //
            m_fses.invoke(new Hangup(m_fses));
            return;
        }
        sipReqParams += ";uuid=";
        sipReqParams += uuid;
        sipReqUri += ";";
        sipReqUri += sipReqParams;
        Set exportVars = new Set(m_fses, m_fses.getVariable("Unique-ID"), "export_vars", "variable_sip_from_uri, Channel-Caller-ID-Number");
        exportVars.start();
        Set hangupAfterBridge = new Set(m_fses, m_fses.getVariable("Unique-ID"), "hangup_after_bridge", "true");
        hangupAfterBridge.start();
        BridgeCommand bridge = new BridgeCommand(m_fses, m_fses.getVariable("Unique-ID"), sipReqUri, domain);
        bridge.start();
    }
 
}
