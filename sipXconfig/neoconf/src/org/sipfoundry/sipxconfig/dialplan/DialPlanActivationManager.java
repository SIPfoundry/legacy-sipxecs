/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.dialplan;

import java.util.Collection;

import org.sipfoundry.sipxconfig.commserver.Location;

public interface DialPlanActivationManager {
    public abstract void replicateDialPlan(boolean restartSbcDevices);
    public abstract void replicateDialPlan(boolean restartSbcDevices, Collection<Location> locations);

    /** Only replicate dial plan if there was a valid request to do so */
    public abstract void replicateIfNeeded();
    /**
     * Replicates the dial plan only on selected location. Note that we don't need to
     * mark services for restart, this is used only on Send profiles action, and the services are
     * restarted automatically.
     * @param restartSbcDevices
     * @param location
     */
    void replicateDialPlan(boolean restartSbcDevices, Location location);
}
