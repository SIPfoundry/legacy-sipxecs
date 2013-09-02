/**
 *
 *
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.common.profile;

import java.io.Serializable;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.commons.userdb.profile.UserProfile;
import org.sipfoundry.commons.userdb.profile.UserProfileService;
import org.sipfoundry.sipxconfig.common.EntityDecorator;
import org.sipfoundry.sipxconfig.common.User;

public class UserDecorator implements EntityDecorator {
    private static final Log LOG = LogFactory.getLog(UserDecorator.class);
    private UserProfileService m_userProfileService;

    public void decorateEntity(Object entity, Serializable id) {
        if (entity instanceof User) {
            try {
                User user = (User) entity;
                UserProfile profile = m_userProfileService.getUserProfile(id.toString());
                if (profile != null) {
                    user.setUserProfile(profile);
                }
            } catch (Exception ex) {
                LOG.error("failed to retrieve user profile " + ex.getMessage());
            }
        }
    }

    public void setUserProfileService(UserProfileService profileService) {
        m_userProfileService = profileService;
    }

    @Override
    public void onSave(Object entity, Serializable id) {
    }

    @Override
    public void onDelete(Object entity, Serializable id) {
    }
}
