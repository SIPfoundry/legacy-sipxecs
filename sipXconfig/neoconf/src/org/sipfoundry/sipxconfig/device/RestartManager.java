/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.device;

import java.util.Collection;
import java.util.Date;

public interface RestartManager {

    /**
     * Restart devices.
     *
     * @param deviceIds collection of phone ids to be restarted
     * @param scheduleTime time at which device should be restarted, null for ASAP
     */
    public void restart(Collection<Integer> deviceIds, Date scheduleTime);

    public void restart(Integer deviceId, Date scheduleTime);
}
