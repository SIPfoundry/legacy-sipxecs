/*
 *
 *
 * Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.openfire;

import org.sipfoundry.sipxconfig.admin.ConfigurationFile;
import org.sipfoundry.sipxconfig.admin.commserver.SipxReplicationContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.event.DaoEventListener;

public class SipxOpenfireDaoListener implements DaoEventListener {

    private SipxReplicationContext m_sipxReplicationContext;
    private ConfigurationFile m_configurationFile;

    public void onDelete(Object entity) {
        if (entity instanceof User) {
            generateOpenfireConfig();
        }
    }

    public void onSave(Object entity) {
        if (entity instanceof User) {
            generateOpenfireConfig();
        }
    }

    /**
     * Regenerate openfire configuration whenever a user is added or removed.
     */
    private void generateOpenfireConfig() {
        m_sipxReplicationContext.replicate(m_configurationFile);
    }

    public void setSipxReplicationContext(SipxReplicationContext sipxReplicationContext) {
        m_sipxReplicationContext = sipxReplicationContext;
    }

    public void setConfigurationFile(ConfigurationFile configurationFile) {
        m_configurationFile = configurationFile;
    }
}
