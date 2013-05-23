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

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.commons.mongo.MongoConstants;
import org.sipfoundry.commons.userdb.profile.UserProfileService;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.springframework.beans.factory.annotation.Required;
import org.springframework.data.mongodb.core.MongoTemplate;
import org.springframework.data.mongodb.core.query.Criteria;
import org.springframework.data.mongodb.core.query.Query;

public class DisableLdapUsersTask implements Runnable {
    public static final String BEAN_NAME = "disableLdapUsersTask";
    public static final Log LOG = LogFactory.getLog(DisableLdapUsersTask.class);
    private UserProfileService m_userProfileService;
    private List<String> m_userNames = new ArrayList<String>();
    private MongoTemplate m_imdb;
    private CoreContext m_coreContext;
    private boolean m_delete;
    private boolean m_disable;

    @Override
    public void run() {
        try {
            if (m_delete) {
                m_coreContext.deleteUsersByUserName(m_userNames);
            } else if (!m_delete && m_disable) {
                m_userProfileService.disableUsers(m_userNames);
                deleteUsersFromMongoImdb(m_userNames);
            }
            m_userNames.clear();
        } catch (Exception ex) {
            if (m_disable) {
                LOG.error("Cannot disable users " + m_userNames, ex);
            } else if (m_delete) {
                LOG.error("Cannot delete users " + m_userNames, ex);
            }
        }
    }

    private void deleteUsersFromMongoImdb(List<String> userNames) {
        m_imdb.remove(new Query(Criteria.where("uid").in(userNames)), MongoConstants.ENTITY_COLLECTION);
    }

    public void addUserName(String userName) {
        m_userNames.add(userName);
    }

    @Required
    public void setUserProfileService(UserProfileService userProfileService) {
        m_userProfileService = userProfileService;
    }

    public boolean isValid() {
        return !m_userNames.isEmpty();
    }

    @Required
    public void setImdb(MongoTemplate imdb) {
        m_imdb = imdb;
    }

    public boolean isDisable() {
        return m_disable;
    }

    public void setDisable(boolean disable) {
        m_disable = disable;
    }

    public boolean isDelete() {
        return m_delete;
    }

    public void setDelete(boolean delete) {
        m_delete = delete;
    }

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }
}
