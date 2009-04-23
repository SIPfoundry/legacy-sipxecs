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

public interface DialPlanActivationManager {
    public abstract void replicateDialPlan(boolean restartSbcDevices);

    /** Only replicate dial plan if there was a valid request to do so */
    public abstract void replicateIfNeeded();
}
