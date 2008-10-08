/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.gateway.audiocodes;

import org.sipfoundry.sipxconfig.phone.PhoneModel;

public class AudioCodesFxsModel extends PhoneModel {
    private String m_configDirectory;
    
    public void setConfigDirectory(String configDirectory) {
        m_configDirectory = configDirectory;
    }

    public String getConfigDirectory() {
        return m_configDirectory;
    }
}
