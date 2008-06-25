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

import org.sipfoundry.sipxconfig.admin.ConfigurationFile;
import org.sipfoundry.sipxconfig.admin.commserver.SipxReplicationContext;
import org.sipfoundry.sipxconfig.admin.dialplan.config.ConfigFileType;

/**
 * Special type profile location that is using replication context to push profiles
 */
public class ReplicatedProfileLocation extends MemoryProfileLocation {

    private SipxReplicationContext m_replicationContext;
    private ConfigFileType m_type;

    public void setReplicationContext(SipxReplicationContext replicationContext) {
        m_replicationContext = replicationContext;
    }

    public void setType(ConfigFileType type) {
        m_type = type;
    }

    public void closeOutput(OutputStream stream) {
        super.closeOutput(stream);
        ConfigurationFile configuration = new InMemoryConfiguration(m_type, toString());
        m_replicationContext.replicate(configuration);
    }

    public void removeProfile(String profileName) {
        super.removeProfile(profileName);
        // FIXME: no way to remove replicated profiles at the moment
    }
}
