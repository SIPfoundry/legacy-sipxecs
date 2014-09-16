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
package org.sipfoundry.openfire.group;

import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.jivesoftware.openfire.XMPPServer;
import org.jivesoftware.openfire.group.Group;
import org.jivesoftware.openfire.group.GroupAlreadyExistsException;
import org.jivesoftware.openfire.group.GroupNotFoundException;
import org.jivesoftware.openfire.provider.GroupProvider;
import org.jivesoftware.util.PersistableMap;
import org.sipfoundry.commons.userdb.UserGroup;
import org.sipfoundry.commons.util.UnfortunateLackOfSpringSupportFactory;
import org.xmpp.packet.JID;

/**
 * Deprecated because of performance issues. Use {@link MongoGroupProviderAlt} instead
 */
@Deprecated
public class MongoGroupProvider implements GroupProvider {
    private final Map<String, String> m_groupMapping = new HashMap<String, String>();

    @Override
    public Group createGroup(String userName) throws UnsupportedOperationException, GroupAlreadyExistsException {
        try {
            return getGroup(userName);
        } catch (GroupNotFoundException ex) {
            throw new UnsupportedOperationException();
        }
    }

    @Override
    public void deleteGroup(String groupName) throws UnsupportedOperationException {
        m_groupMapping.values().remove(groupName);
    }

    @Override
    public Group getGroup(String groupName) throws GroupNotFoundException {
        UserGroup userGroup = UnfortunateLackOfSpringSupportFactory.getValidUsers().getImGroup(groupName);
        if (userGroup == null) {
            throw new GroupNotFoundException("Group with name " + groupName + " not found.");
        }
        List<String> usernames = UnfortunateLackOfSpringSupportFactory.getValidUsers().getImUsernamesInGroup(
                groupName);
        String domain = XMPPServer.getInstance().getServerInfo().getXMPPDomain();
        List<JID> jids = new ArrayList<JID>();
        for (String username : usernames) {
            jids.add(new JID(username + "@" + domain));
        }
        if (userGroup.isImbotEnabled()) {
            String myBuddyName = UnfortunateLackOfSpringSupportFactory.getValidUsers().getImBotName();
            if (myBuddyName != null) {
                jids.add(new JID(myBuddyName + "@" + domain));
            }
        }
        Group openfireGroup = new MongoGroup(userGroup.getGroupName(), userGroup.getDescription(),
                new ArrayList<JID>(), jids);
        openfireGroup.getProperties().put("sharedRoster.showInRoster", "onlyGroup");
        openfireGroup.getProperties().put("sharedRoster.displayName", userGroup.getGroupName());
        m_groupMapping.put(userGroup.getSysId(), userGroup.getGroupName());
        return openfireGroup;
    }

    @Override
    public int getGroupCount() {
        Long count = UnfortunateLackOfSpringSupportFactory.getValidUsers().getImGroupCount();
        return Integer.parseInt(Long.toString(count));
    }

    @Override
    public Collection<String> getGroupNames() {
        return getGroupNames(0, getGroupCount());
    }

    @Override
    public Collection<String> getGroupNames(JID user) {
        String jid = user.toBareJID();
        jid = jid.substring(0, jid.lastIndexOf("@"));
        List<String> names = UnfortunateLackOfSpringSupportFactory.getValidUsers().getImGroupnamesForUser(jid);
        return names;
    }

    @Override
    public Collection<String> getGroupNames(int startIndex, int numResults) {
        return UnfortunateLackOfSpringSupportFactory.getValidUsers().getImGroupNames(startIndex, numResults);
    }

    @Override
    public boolean isReadOnly() {
        return false;
    }

    @Override
    public boolean isSearchSupported() {
        return true;
    }

    @Override
    public Collection<String> search(String query) {
        return search(query, 0, getGroupCount());
    }

    @Override
    public Collection<String> search(String arg0, int arg1, int arg2) {
        return null;
    }

    public String getGroupName(String sysId) {
        return m_groupMapping.get(sysId);
    }

    @Override
    public void addMember(String arg0, JID arg1, boolean arg2) throws UnsupportedOperationException {
        // groups are read-only
    }

    @Override
    public void deleteMember(String arg0, JID arg1) throws UnsupportedOperationException {
        // groups are read-only
    }

    @Override
    public void setDescription(String arg0, String arg1) throws GroupNotFoundException {
        // groups are read-only
    }

    @Override
    public void setName(String arg0, String arg1) throws UnsupportedOperationException, GroupAlreadyExistsException {
        // groups are read-only
    }

    @Override
    public void updateMember(String arg0, JID arg1, boolean arg2) throws UnsupportedOperationException {
        // groups are read-only
    }

    // dummy methods - this class is deprecated, but as long as we keep it
    // around, we need it to compile
    @Override
    public boolean isSharingSupported() {
        return false;
    }

    @Override
    public Collection<String> getSharedGroupNames() {
        return null;
    }

    @Override
    public Collection<String> getSharedGroupNames(JID user) {
        return null;
    }

    @Override
    public Collection<String> getPublicSharedGroupNames() {
        return null;
    }

    @Override
    public Collection<String> getVisibleGroupNames(String userGroup) {
        return null;
    }

    @Override
    public Collection<String> search(String key, String value) {
        return null;
    }

    @Override
    public PersistableMap<String, String> loadProperties(Group group) {
        return null;
    }

    // end dummy methods
}
