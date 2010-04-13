/*
 *
 *
 * Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.service;

import org.sipfoundry.sipxconfig.admin.ConfigurationFile;
import org.sipfoundry.sipxconfig.admin.commserver.SipxReplicationContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.event.DaoEventListener;
import org.springframework.beans.factory.annotation.Required;

public class SipxSaaDaoListener implements DaoEventListener {

    private ConfigurationFile m_configurationFile;

    private SipxReplicationContext m_sipxReplicationContext;

    @Required
    public void setConfigurationFile(ConfigurationFile configurationFile) {
        m_configurationFile = configurationFile;
    }

    @Required
    public void setSipxReplicationContext(SipxReplicationContext sipxReplicationContext) {
        m_sipxReplicationContext = sipxReplicationContext;
    }

    @Override
    public void onDelete(Object entity) {
        if (entity instanceof User) {
            m_sipxReplicationContext.replicate(m_configurationFile);
        }
    }

    @Override
    public void onSave(Object entity) {
        if (entity instanceof User) {
            m_sipxReplicationContext.replicate(m_configurationFile);
        }
    }

}
