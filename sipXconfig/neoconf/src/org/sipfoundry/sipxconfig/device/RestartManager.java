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

public interface RestartManager {

    /**
     * Restart phones in background
     * 
     * @param deviceIds collection of phone ids to be restarted
     */
    public void restart(Collection<Integer> deviceIds);

    public void restart(Integer deviceId);
}
