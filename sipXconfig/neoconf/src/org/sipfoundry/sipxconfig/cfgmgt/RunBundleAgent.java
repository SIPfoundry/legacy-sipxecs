/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.cfgmgt;

import static java.lang.String.format;

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
        String bstr = param(" --bundles ", cfbundles);
        String cstr = param(" --define ", cfclasses);
        String cmd = format("%s %s %s %s", getCommand(), bstr, cstr);
        run(locations, label, cmd);
    }

    String param(String delim, String[] data) {
        return data.length == 0 ? "" : delim + StringUtils.join(data, delim);
    }
}
