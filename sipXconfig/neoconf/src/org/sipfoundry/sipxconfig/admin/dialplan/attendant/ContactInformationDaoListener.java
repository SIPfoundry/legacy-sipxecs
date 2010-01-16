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

import java.util.Map;

import org.sipfoundry.sipxconfig.admin.ConfigurationFile;
import org.sipfoundry.sipxconfig.admin.commserver.SipxReplicationContext;
import org.sipfoundry.sipxconfig.branch.Branch;
import org.sipfoundry.sipxconfig.branch.BranchesWithUsersDeletedEvent;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.event.DaoEventListener;
import org.sipfoundry.sipxconfig.conference.Bridge;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.SettingDao;
import org.springframework.beans.factory.annotation.Required;
import org.springframework.context.ApplicationEvent;
import org.springframework.context.ApplicationListener;

public class ContactInformationDaoListener implements DaoEventListener, ApplicationListener {
    private SipxReplicationContext m_sipxReplicationContext;
    private SettingDao m_settingDao;
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
        if (entity instanceof User) {
            return true;
        } else if (entity instanceof Bridge) {
            return true;
        } else if (entity instanceof Group) {
            Map<Integer, Long> map = m_settingDao.getGroupMemberCountIndexedByGroupId(User.class);
            return map.get(((Group) entity).getId()) != null ? true : false;
        } else if (entity instanceof Branch) {
            Map<Integer, Long> map = m_settingDao.getBranchMemberCountIndexedByBranchId(User.class);
            return map.get(((Branch) entity).getId()) != null ? true : false;
        } else {
            return false;
        }
    }

    public void onApplicationEvent(ApplicationEvent event) {
        if (event instanceof BranchesWithUsersDeletedEvent) {
            m_sipxReplicationContext.replicate(m_configurationFile);
        }
    }

    @Required
    public void setConfigurationFile(ConfigurationFile configurationFile) {
        m_configurationFile = configurationFile;
    }

    @Required
    public void setSipxReplicationContext(SipxReplicationContext sipxReplicationContext) {
        m_sipxReplicationContext = sipxReplicationContext;
    }

    @Required
    public void setSettingDao(SettingDao settingDao) {
        m_settingDao = settingDao;
    }

}
