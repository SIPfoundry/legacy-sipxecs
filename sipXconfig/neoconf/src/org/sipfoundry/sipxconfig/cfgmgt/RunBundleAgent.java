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
package org.sipfoundry.sipxconfig.cfgmgt;

import java.util.Collection;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.commserver.Location;

/**
 * Runs specific CFEngine bundle optionally, with specific CFEngine classes (boolean flags). NOTE
 * you cannot pass general parameters. Security implications among other reasons.  You may be able
 * to send data files on ConfigProvider implementation that these bundles can read and use for input
 */
public class RunBundleAgent extends AgentRunner {

    /**
     * synchronized to ensure cf-agent is run before last one finished, but did not
     * verify this is a strict requirement --Douglas
     */
    public synchronized void run(Collection<Location> locations, String label, String[] cfbundles, String[] cfclasses) {
        String bstr = param(" --bundle ", cfbundles);
        String cstr = param(" --define ", cfclasses);
        run(locations, label, bstr + ' ' + cstr);
    }

    String param(String delim, String[] data) {
        return data.length == 0 ? "" : delim + StringUtils.join(data, delim);
    }
}
