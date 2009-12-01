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
import org.sipfoundry.sipxconfig.conference.Conference;
import org.sipfoundry.sipxconfig.im.ExternalImAccount;
import org.sipfoundry.sipxconfig.setting.Group;

public class SipxOpenfireDaoListener implements DaoEventListener {

    private SipxReplicationContext m_sipxReplicationContext;
    private ConfigurationFile m_configurationFile;

    public void onDelete(Object entity) {
        if (checkGenerateConfig(entity)) {
            generateOpenfireConfig();
        }
    }

    public void onSave(Object entity) {
        if (checkGenerateConfig(entity)) {
            generateOpenfireConfig();
        }
    }

    private boolean checkGenerateConfig(Object entity) {
        if (entity instanceof User || entity instanceof ExternalImAccount) {
            return true;
        }
        if (entity instanceof Group) {
            Group group = (Group) entity;
            return User.GROUP_RESOURCE_ID.equals(group.getResource());
        }
        if (entity instanceof Conference) {
            return true;
        }
        return false;
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
