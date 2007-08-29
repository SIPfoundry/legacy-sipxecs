/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 *
 */
package org.sipfoundry.sipxconfig.vm.attendant;

import org.sipfoundry.sipxconfig.device.FileSystemProfileLocation;
import org.sipfoundry.sipxconfig.device.ProfileGenerator;
import org.sipfoundry.sipxconfig.vm.Mailbox;

public class PersonalAttendantWriter {
    private ProfileGenerator m_generator;
    
    public void write(Mailbox mailbox,  PersonalAttendant pa) {
        FileSystemProfileLocation location = new FileSystemProfileLocation();
        location.setParentDir(mailbox.getUserDirectory().getPath());
        pa.generateProfile(location, m_generator);
    }
    
    public void setGenerator(ProfileGenerator generator) {
        m_generator = generator;
    }
}
