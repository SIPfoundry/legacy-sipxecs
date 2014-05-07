/**
 *
 *
 * Copyright (c) 2013 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.bulk.ldap;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.commons.userdb.profile.UserProfile;
import org.sipfoundry.commons.userdb.profile.UserProfileService;
import org.sipfoundry.sipxconfig.admin.AdminContext;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.beans.factory.annotation.Required;

public class DisableLdapUsersTimer implements BeanFactoryAware {
    public static final Log LOG = LogFactory.getLog(DisableLdapUsersTimer.class);
    private BeanFactory m_beanFactory;
    private UserProfileService m_userProfileService;
    private ExecutorService m_executor = Executors.newSingleThreadExecutor();
    private AdminContext m_adminContext;

    public void disableDeleteUsers() {
        try {
            if (!m_adminContext.isDelete() && !m_adminContext.isDisable()) {
                return;
            }
            DisableLdapUsersTask task = (DisableLdapUsersTask) m_beanFactory.getBean(DisableLdapUsersTask.BEAN_NAME);
            task.setDisable(m_adminContext.isDisable());
            task.setDelete(m_adminContext.isDelete());
            List<UserProfile> userProfiles = new ArrayList<UserProfile>();
            if (task.isDelete()) {
                userProfiles = m_userProfileService.
                        getUserProfilesToDelete(m_adminContext.getAge() * 60 * 60 * 1000);
                if (userProfiles.size() > 0) {
                    LOG.info("Users number to delete: "
                        + userProfiles.size() + " and with age: " + m_adminContext.getAge());
                }

            } else if (!task.isDelete() && task.isDisable()) {
                userProfiles = m_userProfileService.
                    getUserProfilesToDisable(m_adminContext.getAge() * 60 * 60 * 1000);
                if (userProfiles.size() > 0) {
                    LOG.info("Users number to disable: "
                        + userProfiles.size() + " with age: " + m_adminContext.getAge());
                }
            }
            for (UserProfile profile : userProfiles) {
                task.addUserName(profile.getUserName());
            }
            if (task.isValid()) {
                m_executor.execute(task);
            }
        } catch (Exception ex) {
            LOG.error("Failed running disabled/delete task ", ex);
        }
    }

    @Override
    @Required
    public void setBeanFactory(BeanFactory beanFactory) {
        m_beanFactory = beanFactory;
    }

    @Required
    public void setUserProfileService(UserProfileService userProfileService) {
        m_userProfileService = userProfileService;
    }

    @Required
    public void setAdminContext(AdminContext adminContext) {
        m_adminContext = adminContext;
    }
}
