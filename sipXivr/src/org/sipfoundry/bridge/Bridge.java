/**
 *
 *
 * Copyright (c) 2010 / 2011 eZuce, Inc. All rights reserved.
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

import org.apache.log4j.Logger;
import org.sipfoundry.sipxivr.SipxIvrApp;


public class Bridge extends SipxIvrApp {
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxivr");

    /**
     * Run the bridge. Bridge the call back to freeswitch with all the parameters intact
     *
     * @throws Throwable indicating an error or hangup condition.
     */
    @Override
    public void run() {
        BridgeEslRequestController controller = (BridgeEslRequestController) getEslRequestController();
        controller.set("export_vars", "variable_sip_from_uri, Channel-Caller-ID-Number");
        controller.bridgeCall();
    }
}
