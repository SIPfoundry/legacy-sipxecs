/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.service;

import org.sipfoundry.sipxconfig.setting.Setting;

public class UnmanagedService extends ConfiguredService {
    public static final String BEAN_ID = "unmanagedService";
    
    public static final ServiceDescriptor NTP = new ServiceDescriptor(BEAN_ID, "ntpService", "NTP");
    
    public UnmanagedService() {        
        super(BEAN_ID);
    }
    
    public final ServiceDescriptor ntp() {
        return NTP;
    }

    @Override
    protected Setting loadSettings() {
        return null;
    }
}
