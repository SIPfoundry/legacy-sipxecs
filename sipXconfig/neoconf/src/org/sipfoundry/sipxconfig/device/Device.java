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

public interface Device {

    void setProfileGenerator(ProfileGenerator profileGenerator);

    void generateProfiles();

    void removeProfiles();

    String getSerialNumber();
}
