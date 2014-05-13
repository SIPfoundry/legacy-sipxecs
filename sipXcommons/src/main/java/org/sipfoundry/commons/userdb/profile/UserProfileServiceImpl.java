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
import java.util.ArrayList;
import java.util.Date;
import java.util.List;

import javax.imageio.ImageIO;

import net.coobird.thumbnailator.Thumbnails;
import net.coobird.thumbnailator.geometry.Positions;

import org.apache.commons.io.IOUtils;
import org.apache.commons.lang.StringUtils;
import org.bson.types.ObjectId;
import org.springframework.data.mongodb.core.MongoTemplate;
import org.springframework.data.mongodb.core.query.Criteria;
import org.springframework.data.mongodb.core.query.Query;
import org.springframework.data.mongodb.core.query.Update;

import com.mongodb.gridfs.GridFS;
import com.mongodb.gridfs.GridFSDBFile;
import com.mongodb.gridfs.GridFSInputFile;

public class UserProfileServiceImpl implements UserProfileService {
    private static final String USER_PROFILE_COLLECTION = "userProfile";
    private static final String USER_ID = "_id";
    private static final String IM_ID = "m_imId";
    private static final String AUTH_ACCOUNT_NAME = "m_authAccountName";
    private static final String PRIMARY_EMAIL = "m_emailAddress";
    private static final String ALTERNATE_EMAIL = "m_alternateEmailAddress";
    private static final String ALIASES_EMAIL_SET = "m_emailAddressAliasesSet";
    private static final String USERNAME = "m_userName";
    private static final String BRANCH_NAME = "m_branchName";
    private static final String BRANCH_ADDRESS = "m_branchAddress";
    private static final String AVATAR_NAME = "avatar_%s.png";
    private static final int LIMIT_DISABLE = 500;
    private static final int LIMIT_DELETE = 50;
    private static String m_defaultId = "";
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
            return m_template.findOne(new Query(Criteria.where(IM_ID).is(imId.toLowerCase())), UserProfile.class,
                    USER_PROFILE_COLLECTION);
        }
        return null;
    }

    @Override
    public List<UserProfile> getUserProfileByAuthAccountName(String authAccountName) {
        if (authAccountName != null) {
            return m_template.find(new Query(Criteria.where(AUTH_ACCOUNT_NAME).is(authAccountName.toLowerCase())),
                    UserProfile.class, USER_PROFILE_COLLECTION);
        }
        return null;
    }

    @Override
    public List<UserProfile> getUserProfileByEmail(String email) {
        if (email != null) {
            Query query = new Query(new Criteria().orOperator(Criteria.where(PRIMARY_EMAIL).is(email.toLowerCase()),
                    Criteria.where(ALTERNATE_EMAIL).is(email.toLowerCase()),
                    Criteria.where(ALIASES_EMAIL_SET).is(email.toLowerCase())));
            return m_template.find(query, UserProfile.class, USER_PROFILE_COLLECTION);
        }
        return null;
    }

    @Override
    public List<Integer> getUserIdsByAuthAccountName(String authAccountName) {
        List<Integer> ids = new ArrayList<Integer>();
        List<UserProfile> userProfiles = getUserProfileByAuthAccountName(authAccountName);
        for (UserProfile userProfile : userProfiles) {
            ids.add(Integer.valueOf(userProfile.getUserId()));
        }
        return ids;
    }

    @Override
    public List<Integer> getUserIdsByEmail(String email) {
        List<Integer> ids = new ArrayList<Integer>();
        List<UserProfile> userProfiles = getUserProfileByEmail(email);
        for (UserProfile userProfile : userProfiles) {
            ids.add(Integer.valueOf(userProfile.getUserId()));
        }
        return ids;
    }

    @Override
    public List<UserProfile> getAllUserProfiles() {
        return m_template.findAll(UserProfile.class);
    }

    @Override
    public InputStream getAvatar(String userName) {
        GridFSDBFile image = getAvatarDBFile(userName);
        if (image != null) {
            return image.getInputStream();
        }
        return null;
    }

    @Override
    public ObjectId getAvatarId(String userName) {
        GridFSDBFile imageForOutput = getAvatarDBFile(userName);
        if (imageForOutput != null) {
            return (ObjectId) getAvatarDBFile(userName).getId();
        }

        return null;
    }

    private GridFSDBFile getAvatarDBFile(String userName) {
        GridFS avatarFS = new GridFS(m_template.getDb());
        GridFSDBFile imageForOutput = avatarFS.findOne(String.format(AVATAR_NAME, userName));
        if (imageForOutput != null) {
            return imageForOutput;
        }
        // try default avatar
        imageForOutput = avatarFS.findOne(String.format(AVATAR_NAME, "default"));
        if (imageForOutput != null) {
            m_defaultId = imageForOutput.getId().toString();
            return imageForOutput;
        }
        return null;
    }
    @Override
    public String getAvatarDBFileMD5(String userName) {
        GridFSDBFile avatar = getAvatarDBFile(userName);
        return avatar != null ? avatar.getMD5() : null;
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
            BufferedImage thumbnail = Thumbnails.of(originalImage).crop(Positions.CENTER).size(128, 128)
                    .asBufferedImage();
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
    public void saveAvatar(String userName, InputStream originalIs, boolean overwriteIfExists)
            throws AvatarUploadException {
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

    @Override
    public void disableUsers(List<String> userNames) {
        m_template.updateMulti(new Query(Criteria.where("m_userName").in(userNames)),
                new Update().set("m_enabled", false).set("m_disabledDate", new Date()), USER_PROFILE_COLLECTION);
    }

    @Override
    public List<UserProfile> getUserProfilesToDisable(long age) {
        return m_template.find(
                new Query(Criteria.where("m_lastImportedDate").lt(new Date(new Date().getTime() - age))
                        .and("m_enabled").is(true).and("m_ldapManaged").is(true)).limit(LIMIT_DISABLE),
                UserProfile.class, USER_PROFILE_COLLECTION);
    }

    @Override
    public List<UserProfile> getUserProfilesToDelete(long age) {
        return m_template.find(
                new Query(Criteria.where("m_lastImportedDate").lt(new Date(new Date().getTime() - age))
                        .and("m_ldapManaged").is(true)).limit(LIMIT_DELETE), UserProfile.class,
                USER_PROFILE_COLLECTION);
    }

    public void setProfilesDb(MongoTemplate template) {
        m_template = template;
    }

    @Override
    public List<UserProfile> getUserProfilesByEnabledProperty(String search, int firstRow, int pageSize) {
        // default search enabled users
        boolean enabled = true;
        String property = "m_enabled";
        if (StringUtils.equals(search, DISABLED)) {
            enabled = false;
        }
        if (StringUtils.equals(search, LDAP)) {
            property = "m_ldapManaged";
        }
        return getUserProfiles(firstRow, pageSize, property, enabled);
    }

    private List<UserProfile> getUserProfiles(int firstRow, int pageSize, String property, boolean enabled) {
        return m_template.find(new Query(Criteria.where(property).is(enabled)).skip(firstRow).limit(pageSize),
                UserProfile.class, USER_PROFILE_COLLECTION);
    }

    @Override
    public int getEnabledUsersCount() {
        return countUsers(true).intValue();
    }

    @Override
    public int getDisabledUsersCount() {
        return countUsers(false).intValue();
    }

    private Long countUsers(boolean active) {
        return m_template.count(new Query(Criteria.where("m_enabled").is(active)), USER_PROFILE_COLLECTION);
    }

    public static String getDefaultId() {
        return m_defaultId;
    }
}
