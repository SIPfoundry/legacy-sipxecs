/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.dialplan;

import java.util.Collection;

import org.sipfoundry.sipxconfig.admin.commserver.Location;

public interface DialPlanActivationManager {
    public abstract void replicateDialPlan(boolean restartSbcDevices);
    public abstract void replicateDialPlan(boolean restartSbcDevices, Collection<Location> locations);

    /** Only replicate dial plan if there was a valid request to do so */
    public abstract void replicateIfNeeded();
}
