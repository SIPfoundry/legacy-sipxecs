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

import java.io.IOException;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.commons.userdb.profile.AvatarUploadException;
import org.sipfoundry.commons.userdb.profile.UserProfile;
import org.sipfoundry.commons.userdb.profile.UserProfileService;
import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.apache.ApacheManager;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.common.event.DaoEventListener;
import org.springframework.core.io.ClassPathResource;
import org.springframework.core.io.Resource;

public class UserProfileContext implements DaoEventListener {
    private static final Log LOG = LogFactory.getLog(UserProfileContext.class);
    private static final String AVATAR_FORMAT = "%s/sipxconfig/rest/avatar/%s";
    private UserProfileService m_userProfileService;
    private AddressManager m_addressManager;

    public void uploadDefaultAvatar() {
        try {
            Resource defaultAvatar = new ClassPathResource(
                    "org/sipfoundry/sipxconfig/common/profile/default_avatar.jpg");
            m_userProfileService.saveAvatar("default", defaultAvatar.getInputStream(), false);
        } catch (AvatarUploadException e) {
            LOG.error("failed to upload default avatar " + e.getMessage());
        } catch (IOException e) {
            LOG.error("failed to retrieve default avatar " + e.getMessage());
        }
    }

    @Override
    public void onDelete(Object entity) {
        if (entity instanceof User) {
            try {
                User user = (User) entity;
                m_userProfileService.deleteUserProfile(user.getUserProfile());
            } catch (Exception ex) {
                LOG.error("failed to delete profile from mongo " + ex.getMessage());
                throw new UserException("&err.msg.deleteProfile", ex.getMessage());
            }
        }
    }

    @Override
    public void onSave(Object entity) {
        if (entity instanceof User) {
            try {
                User user = (User) entity;
                UserProfile profile = user.getUserProfile();
                Address apacheAddr = m_addressManager.getSingleAddress(ApacheManager.HTTPS_ADDRESS);
                if (apacheAddr != null) {
                    profile.setAvatar(String.format(AVATAR_FORMAT, apacheAddr.toString(), user.getUserName()));
                }
                m_userProfileService.saveUserProfile(profile);
            } catch (Exception ex) {
                LOG.error("failed to save profile in mongo" + ex.getMessage());
                throw new UserException("&err.msg.saveProfile", ex.getMessage());
            }
        }
    }

    public void setUserProfileService(UserProfileService profileService) {
        m_userProfileService = profileService;
    }

    public void setAddressManager(AddressManager manager) {
        m_addressManager = manager;
    }

}
