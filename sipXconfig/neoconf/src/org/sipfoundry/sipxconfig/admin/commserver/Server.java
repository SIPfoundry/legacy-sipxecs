/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.admin.commserver;

import org.sipfoundry.sipxconfig.setting.Setting;

public interface Server {
    Setting getSettings();
    
    String getPresenceServerUri();

    String getPresenceServiceUri();
    
    String getPagingLogLevel();

    /**
     * Saves settings to permanent storage (config files)
     */
    void applySettings();
    
    /**
     * Clears unapplied values, does not resets everything to default.
     */
    void resetSettings();
}
