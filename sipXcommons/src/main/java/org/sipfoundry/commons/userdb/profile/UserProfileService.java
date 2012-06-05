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
package org.sipfoundry.commons.userdb.profile;

import java.io.InputStream;
import java.util.List;

public interface UserProfileService {
    UserProfile getUserProfile(String userId);

    void saveUserProfile(UserProfile profile);

    void deleteUserProfile(UserProfile profile);

    void deleteUserProfile(String userName);

    Integer getUserIdByImId(String imId);

    boolean isImIdInUse(String imId, String username);

    boolean isImIdInUse(String imId);

    String getUsernameByImId(String imId);

    UserProfile getUserProfileByImId(String imId);

    InputStream getAvatar(String userName);

    void saveAvatar(String userName, InputStream is) throws AvatarUploadException;

    void saveAvatar(String userName, InputStream is, boolean overwrite) throws AvatarUploadException;

    void deleteAvatar(String userName);

    List<UserProfile> getAllUserProfiles();

}
