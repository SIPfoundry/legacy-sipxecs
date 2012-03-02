/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.cfgmgt;


import java.util.Collection;

import org.sipfoundry.sipxconfig.commserver.Location;

public class ConfigAgent extends AgentRunner {

    /**
     * synchronized to ensure cf-agent is run before last one finished, but did not
     * verify this is a strict requirement --Douglas
     */
    public synchronized void run(Collection<Location> locations) {
        run(locations, "Configuration deployment", "");
    }
}
