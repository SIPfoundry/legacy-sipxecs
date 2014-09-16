/**
 *
 *
 * Copyright (c) 2014 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.openfire.user;

import static org.apache.commons.lang.StringUtils.defaultIfEmpty;
import static org.apache.commons.lang.StringUtils.lowerCase;
import static org.sipfoundry.commons.mongo.MongoConstants.UID;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.Date;
import java.util.HashMap;
import java.util.LinkedHashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.jivesoftware.openfire.XMPPServer;
import org.jivesoftware.openfire.provider.UserProvider;
import org.jivesoftware.openfire.user.User;
import org.jivesoftware.openfire.user.UserAlreadyExistsException;
import org.jivesoftware.openfire.user.UserCollection;
import org.jivesoftware.openfire.user.UserNotFoundException;
import org.sipfoundry.commons.userdb.ValidUsers;
import org.sipfoundry.commons.util.UnfortunateLackOfSpringSupportFactory;
import org.sipfoundry.openfire.provider.MongoAuthProvider;
import org.xmpp.packet.JID;

/**
 * Deprecated because of performance issues. Use {@link MongoUserpProviderAlt} instead
 */
@Deprecated
public class MongoUserProvider implements UserProvider {
    public static final String SIP_UID = "sipUid";

    private final Map<String, String> m_userMapping = new HashMap<String, String>();

    public MongoUserProvider() {
        // Executors.newFixedThreadPool(1).execute(new MongoOplogListener());
    }

    @Override
    public User createUser(String username, String password, String name, String email)
            throws UserAlreadyExistsException {
        try {
            return loadUser(username);
        } catch (UserNotFoundException ex) {
            throw new UnsupportedOperationException();
        }
    }

    @Override
    public void deleteUser(String username) {
        m_userMapping.values().remove(username);
    }

    @Override
    public Collection<User> findUsers(Set<String> fields, String query) throws UnsupportedOperationException {
        return findUsers(fields, query, 0, getUserCount());
    }

    @Override
    public Collection<User> findUsers(Set<String> fields, String query, int startIndex, int numResults)
            throws UnsupportedOperationException {
        if (fields.isEmpty()) {
            return Collections.emptyList();
        }
        if (!getSearchFields().containsAll(fields)) {
            throw new IllegalArgumentException("Search fields " + fields + " are not valid.");
        }
        if (query == null || "".equals(query)) {
            return Collections.emptyList();
        }
        List<org.sipfoundry.commons.userdb.User> users = UnfortunateLackOfSpringSupportFactory.getValidUsers()
                .getImUsersByFilter(fields, query, startIndex, numResults);
        List<User> ofUsers = new ArrayList<User>();
        for (org.sipfoundry.commons.userdb.User user : users) {
            User ofUser = getOfUser(user);
            ofUser.setNameVisible(true);
            ofUsers.add(ofUser);
        }
        return ofUsers;
    }

    @Override
    public Set<String> getSearchFields() throws UnsupportedOperationException {
        return new LinkedHashSet<String>(Arrays.asList(ValidUsers.IM_USERNAME_FILTER, ValidUsers.IM_NAME_FILTER,
                ValidUsers.IM_EMAIL_FILTER));
    }

    @Override
    public int getUserCount() {
        Long count = UnfortunateLackOfSpringSupportFactory.getValidUsers().getImUsersCount();
        return Integer.parseInt(Long.toString(count));
    }

    @Override
    public Collection<String> getUsernames() {
        return getUsernames(0, getUserCount());
    }

    @Override
    public Collection<User> getUsers() {
        Collection<String> usernames = getUsernames(0, getUserCount());
        return new UserCollection(usernames.toArray(new String[usernames.size()]));
    }

    @Override
    public Collection<User> getUsers(int start, int end) {
        Collection<String> usernames = getUsernames(start, end);
        return new UserCollection(usernames.toArray(new String[usernames.size()]));
    }

    private static Collection<String> getUsernames(int startIndex, int numResults) {
        return UnfortunateLackOfSpringSupportFactory.getValidUsers().getImUsernames(startIndex, numResults);
    }

    @Override
    public boolean isEmailRequired() {
        return false;
    }

    @Override
    public boolean isNameRequired() {
        return false;
    }

    @Override
    public boolean isReadOnly() {
        return false;
    }

    @Override
    public User loadUser(String username) throws UserNotFoundException {
        if (username.contains("@")) {
            if (!XMPPServer.getInstance().isLocal(new JID(username))) {
                throw new UserNotFoundException("Cannot load user of remote server: " + username);
            }
            username = username.substring(0, username.lastIndexOf("@"));
        }
        org.sipfoundry.commons.userdb.User user = UnfortunateLackOfSpringSupportFactory.getValidUsers()
                .getUserByInsensitiveJid(username);
        if (user == null || (!user.getUserName().equals(MongoAuthProvider.SUPERADMIN) && !user.isImEnabled())) {
            throw new UserNotFoundException("user not found");
        }
        m_userMapping.put(user.getSysId(), user.getJid());
        return getOfUser(user);
    }

    private static User getOfUser(org.sipfoundry.commons.userdb.User user) {
        User ofUser = new MongoUser(lowerCase(user.getJid()), user.getImDisplayName(), user.getEmailAddress(),
                new Date(), new Date());
        ofUser.setNameVisible(true);
        ofUser.getProperties().put(UID, defaultIfEmpty(user.getUserName(), ""));
        ofUser.getProperties().put(SIP_UID, user.getUserName());
        return ofUser;
    }

    @Override
    public void setCreationDate(String arg0, Date arg1) throws UserNotFoundException {
        // user is read-only
    }

    @Override
    public void setEmail(String arg0, String arg1) throws UserNotFoundException {
        // user is read-only
    }

    @Override
    public void setModificationDate(String arg0, Date arg1) throws UserNotFoundException {
        // user is read-only
    }

    @Override
    public void setName(String arg0, String arg1) throws UserNotFoundException {
        // user is read-only
    }

    public String getUserName(String sysId) {
        return m_userMapping.get(sysId);
    }
}
