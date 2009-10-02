/*
 *
 *
 * Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.admin.dialplan.attendant;

import org.sipfoundry.sipxconfig.admin.ConfigurationFile;
import org.sipfoundry.sipxconfig.admin.commserver.SipxReplicationContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.event.DaoEventListener;
import org.sipfoundry.sipxconfig.conference.Bridge;
import org.springframework.beans.factory.annotation.Required;

public class ContactInformationDaoListener implements DaoEventListener {
    private SipxReplicationContext m_sipxReplicationContext;
    private ConfigurationFile m_configurationFile;

    @Override
    public void onDelete(Object entity) {
        generate(entity);
    }

    @Override
    public void onSave(Object entity) {
        generate(entity);
    }

    private void generate(Object entity) {
        if (shouldGenerate(entity)) {
            m_sipxReplicationContext.replicate(m_configurationFile);
        }
    }

    private boolean shouldGenerate(Object entity) {
        return entity instanceof User || entity instanceof Bridge;
    }

    @Required
    public void setConfigurationFile(ConfigurationFile configurationFile) {
        m_configurationFile = configurationFile;
    }

    @Required
    public void setSipxReplicationContext(SipxReplicationContext sipxReplicationContext) {
        m_sipxReplicationContext = sipxReplicationContext;
    }
}
