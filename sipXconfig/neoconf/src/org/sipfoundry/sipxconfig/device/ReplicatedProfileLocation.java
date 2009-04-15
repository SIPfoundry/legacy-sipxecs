/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.device;

import java.io.OutputStream;

import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.SipxReplicationContext;
import org.springframework.beans.factory.annotation.Required;

/**
 * Special type profile location that is using replication context to push profiles
 */
public class ReplicatedProfileLocation extends MemoryProfileLocation {

    private SipxReplicationContext m_replicationContext;
    private String m_directory;
    private String m_name;
    private Location m_location;

    @Required
    public void setReplicationContext(SipxReplicationContext replicationContext) {
        m_replicationContext = replicationContext;
    }

    @Required
    public void setName(String name) {
        m_name = name;
    }

    @Required
    public void setDirectory(String directory) {
        m_directory = directory;
    }

    public void setLocation(Location location) {
        m_location = location;
    }

    @Override
    public void closeOutput(OutputStream stream) {
        super.closeOutput(stream);
        InMemoryConfiguration configuration = new InMemoryConfiguration(m_directory, m_name, toString());
        configuration.setLocation(m_location);
        m_replicationContext.replicate(configuration);
    }

    @Override
    public void removeProfile(String profileName) {
        super.removeProfile(profileName);
        // FIXME: no way to remove replicated profiles at the moment
    }
}
