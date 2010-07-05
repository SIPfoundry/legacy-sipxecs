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

import java.util.List;

import org.sipfoundry.sipxconfig.admin.ConfigurationFile;
import org.sipfoundry.sipxconfig.admin.commserver.SipxReplicationContext;
import org.sipfoundry.sipxconfig.admin.localization.Localization;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.event.DaoEventListener;
import org.sipfoundry.sipxconfig.conference.Conference;
import org.sipfoundry.sipxconfig.service.ServiceConfigurator;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;
import org.sipfoundry.sipxconfig.setting.Group;
import org.springframework.beans.factory.annotation.Required;

public class SipxOpenfireDaoListener implements DaoEventListener {

    private SipxReplicationContext m_sipxReplicationContext;
    private SipxServiceManager m_sipxServiceManager;
    private List<ConfigurationFile> m_configurationFiles;
    private ServiceConfigurator m_serviceConfigurator;

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
        if (entity instanceof User) {
            return true;
        }
        if (entity instanceof Group) {
            Group group = (Group) entity;
            return User.GROUP_RESOURCE_ID.equals(group.getResource());
        }
        if (entity instanceof Conference) {
            return true;
        }
        if (entity instanceof Localization) {
            Localization locale = (Localization) entity;
            SipxOpenfireService instantMessagingService = (SipxOpenfireService) m_sipxServiceManager
                    .getServiceByBeanId("sipxOpenfireService");
            instantMessagingService.setLocale(locale.getLanguage());
            m_serviceConfigurator.markServiceForRestart(instantMessagingService);
            return true;
        }
        return false;
    }

    /**
     * Regenerate openfire configuration whenever a user is added or removed.
     */
    private void generateOpenfireConfig() {
        for (ConfigurationFile configurationFile : m_configurationFiles) {
            m_sipxReplicationContext.replicate(configurationFile);
        }
    }

    public void setSipxReplicationContext(SipxReplicationContext sipxReplicationContext) {
        m_sipxReplicationContext = sipxReplicationContext;
    }

    public void setConfigurationFiles(List<ConfigurationFile> configurationFiles) {
        m_configurationFiles = configurationFiles;
    }

    @Required
    public void setSipxServiceManager(SipxServiceManager sipxServiceManager) {
        m_sipxServiceManager = sipxServiceManager;
    }

    public void setServiceConfigurator(ServiceConfigurator serviceConfigurator) {
        m_serviceConfigurator = serviceConfigurator;
    }
}
