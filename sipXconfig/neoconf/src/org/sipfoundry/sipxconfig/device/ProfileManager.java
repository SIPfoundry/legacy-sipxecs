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

public interface ProfileManager {
    /**
     * Generate profile on phones in background
     * 
     * @param deviceIds collection of ids of device (phone or gateway) objects
     */
    public void generateProfilesAndRestart(Collection deviceIds);

    public void generateProfileAndRestart(Integer deviceId);
}
