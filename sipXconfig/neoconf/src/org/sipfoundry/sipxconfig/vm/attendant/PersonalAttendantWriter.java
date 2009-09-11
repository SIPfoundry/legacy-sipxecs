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

import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.device.FileSystemProfileLocation;
import org.sipfoundry.sipxconfig.device.ProfileGenerator;
import org.sipfoundry.sipxconfig.vm.Mailbox;

public class PersonalAttendantWriter {
    private ProfileGenerator m_generator;

    private CoreContext m_coreContext;

    public void write(Mailbox mailbox, PersonalAttendant pa) {
        FileSystemProfileLocation location = new FileSystemProfileLocation();
        location.setParentDir(mailbox.getUserDirectory().getPath());
        String domain = m_coreContext.getDomainName();
        pa.generatePropertiesProfile(location, domain, m_generator);
    }

    public void setGenerator(ProfileGenerator generator) {
        m_generator = generator;
    }

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }
}
