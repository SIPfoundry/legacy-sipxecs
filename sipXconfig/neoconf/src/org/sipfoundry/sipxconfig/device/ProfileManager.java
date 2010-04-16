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

public interface ProfileManager {
    /**
     * Generate profile on phones in background, optionally restart the devices for which profile
     * has been generated. Restart is triggered only after profiles are generated.
     *
     * @param deviceIds collection of ids of device (phone or gateway) objects
     * @param restart if true restart the device
     * @param time at which devices should be restarted - ignored unless restart is true, if
     *        restart if true and restartTime is null devices will be restarted as soon as
     *        possible, but only after profiles are generated
     */
    public void generateProfiles(Collection<Integer> deviceIds, boolean restart, Date restartTime);

    public void generateProfile(Integer deviceId, boolean restart, Date restartTime);

    public void restartDevices(Collection<Integer> devices, Date restartTime);

    public void restartDevice(Integer device, Date restartTime);
}
