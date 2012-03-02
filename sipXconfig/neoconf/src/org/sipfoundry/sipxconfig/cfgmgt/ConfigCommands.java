/*
 * Copyright (C) 2012 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.cfgmgt;

import java.util.Collection;

import org.sipfoundry.sipxconfig.commserver.Location;


/**
 * Effectively run config related commands at given locations. It ultimately calls
 * a combination of CFEngine bundles and/or defines.
 */
public interface ConfigCommands {

    public void restartServices();

    public void restartServices(Collection<Location> locations);

    public void lastSeen();
}
