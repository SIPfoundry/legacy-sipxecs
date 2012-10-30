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

import java.awt.image.BufferedImage;
import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.InputStream;
import java.util.List;

import javax.imageio.ImageIO;

import net.coobird.thumbnailator.Thumbnails;
import net.coobird.thumbnailator.geometry.Positions;

import org.apache.commons.io.IOUtils;
import org.springframework.data.mongodb.core.MongoTemplate;
import org.springframework.data.mongodb.core.query.Criteria;
import org.springframework.data.mongodb.core.query.Query;
import org.springframework.data.mongodb.core.query.Update;

import com.mongodb.gridfs.GridFS;
import com.mongodb.gridfs.GridFSDBFile;
import com.mongodb.gridfs.GridFSInputFile;

public class UserProfileServiceImpl implements UserProfileService {
    private static final String USER_PROFILE_COLLECTION = "userProfile";
    private static final String USER_ID = "m_userid";
    private static final String IM_ID = "m_imId";
    private static final String USERNAME = "m_userName";
    private static final String BRANCH_NAME = "m_branchName";
    private static final String BRANCH_ADDRESS = "m_branchAddress";
    private static final String AVATAR_NAME = "avatar_%s.png";
    private MongoTemplate m_template;

    @Override
    public UserProfile getUserProfile(String userId) {
        return m_template.findOne(new Query(Criteria.where(USER_ID).is(userId.toString())), UserProfile.class,
                USER_PROFILE_COLLECTION);
    }

    @Override
    public void saveUserProfile(UserProfile profile) {
        m_template.save(profile);
    }

    @Override
    public void deleteUserProfile(UserProfile profile) {
        m_template.remove(profile);
        deleteAvatar(profile.getUserName());
    }

    @Override
    public void deleteUserProfile(String userName) {
        m_template.findAndRemove(new Query(Criteria.where(USERNAME).is(userName)), UserProfile.class);
        deleteAvatar(userName);
    }

    @Override
    public Integer getUserIdByImId(String imId) {
        UserProfile userProfile = getUserProfileByImId(imId);
        if (userProfile != null) {
            return Integer.valueOf(userProfile.getUserId());
        }
        return null;
    }

    @Override
    public boolean isAliasInUse(String alias, String username) {
        UserProfile userProfile = getUserProfileByImId(alias);
        if (userProfile != null && !userProfile.getUserName().equals(username)) {
            return true;
        }
        return false;
    }

    @Override
    public boolean isImIdInUse(String imId, Integer userId) {
        UserProfile userProfile = getUserProfileByImId(imId);
        if (userProfile != null && !userProfile.getUserId().equals(userId.toString())) {
            return true;
        }
        return false;
    }

    @Override
    public boolean isImIdInUse(String imId) {
        UserProfile userProfile = getUserProfileByImId(imId);
        if (userProfile != null) {
            return true;
        }
        return false;
    }

    @Override
    public String getUsernameByImId(String imId) {
        UserProfile userProfile = getUserProfileByImId(imId);
        if (userProfile != null) {
            return userProfile.getUserName();
        }
        return null;
    }

    @Override
    public UserProfile getUserProfileByImId(String imId) {
        if (imId != null) {
            return m_template.findOne(new Query(Criteria.where(IM_ID).is(imId.toLowerCase())), UserProfile.class, USER_PROFILE_COLLECTION);
        }
        return null;
    }

    @Override
    public List<UserProfile> getAllUserProfiles() {
        return m_template.findAll(UserProfile.class);
    }

    @Override
    public InputStream getAvatar(String userName) {
        GridFS avatarFS = new GridFS(m_template.getDb());
        GridFSDBFile imageForOutput = avatarFS.findOne(String.format(AVATAR_NAME, userName));
        if (imageForOutput != null) {
            return imageForOutput.getInputStream();
        }
        // try default avatar
        imageForOutput = avatarFS.findOne(String.format(AVATAR_NAME, "default"));
        if (imageForOutput != null) {
            return imageForOutput.getInputStream();
        }
        return null;
    }

    @Override
    public void deleteAvatar(String userName) {
        GridFS avatarFS = new GridFS(m_template.getDb());
        avatarFS.remove(String.format(AVATAR_NAME, userName));
    }

    @Override
    public void saveAvatar(String userName, InputStream originalIs) throws AvatarUploadException {
        ByteArrayOutputStream os = null;
        InputStream is = null;
        try {
            BufferedImage originalImage = ImageIO.read(originalIs);
            BufferedImage thumbnail = Thumbnails.of(originalImage).crop(Positions.CENTER).size(128, 128).asBufferedImage();
            os = new ByteArrayOutputStream();
            ImageIO.write(thumbnail, "png", os);
            is = new ByteArrayInputStream(os.toByteArray());
            String fileName = String.format(AVATAR_NAME, userName);
            GridFS avatarFS = new GridFS(m_template.getDb());
            avatarFS.remove(fileName);
            GridFSInputFile gfsFile = avatarFS.createFile(is);
            gfsFile.setFilename(fileName);
            gfsFile.save();
        } catch (Exception ex) {
            throw new AvatarUploadException(ex);
        } finally {
            IOUtils.closeQuietly(originalIs);
            IOUtils.closeQuietly(is);
            IOUtils.closeQuietly(os);
        }
    }

    @Override
    public void saveAvatar(String userName, InputStream originalIs, boolean overwriteIfExists) throws AvatarUploadException {
        GridFS avatarFS = new GridFS(m_template.getDb());
        GridFSDBFile imageForOutput = avatarFS.findOne(String.format(AVATAR_NAME, userName));
        if (imageForOutput == null || (imageForOutput != null && overwriteIfExists)) {
            saveAvatar(userName, originalIs);
        }
    }

    @Override
    public void updateBranchAddress(String branchName, Address address) {
        m_template.updateMulti(new Query(Criteria.where(BRANCH_NAME).is(branchName)),
                new Update().set(BRANCH_ADDRESS, address), USER_PROFILE_COLLECTION);
    }

    public void setProfilesDb(MongoTemplate template) {
        m_template = template;
    }
}
