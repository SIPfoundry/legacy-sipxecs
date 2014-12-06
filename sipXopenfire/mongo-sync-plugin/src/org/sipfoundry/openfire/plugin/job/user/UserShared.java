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
package org.sipfoundry.openfire.plugin.job.user;

import java.util.Collection;

import org.apache.commons.lang.StringUtils;
import org.apache.log4j.Logger;
import org.jivesoftware.openfire.SessionManager;
import org.jivesoftware.openfire.XMPPServer;
import org.jivesoftware.openfire.group.Group;
import org.jivesoftware.openfire.group.GroupManager;
import org.jivesoftware.openfire.group.GroupNotFoundException;
import org.jivesoftware.openfire.session.ClientSession;
import org.jivesoftware.openfire.user.User;
import org.jivesoftware.openfire.user.UserManager;
import org.xmpp.packet.JID;
import org.xmpp.packet.StreamError;

public class UserShared {
    private static Logger logger = Logger.getLogger(UserShared.class);

    public static void removeUser(String userImName, Collection<String> userGroups) {
        logger.debug("Close deleted user xmpp session");
        final StreamError error = new StreamError(StreamError.Condition.not_authorized);
        for (ClientSession sess : SessionManager.getInstance().getSessions(userImName)) {
            sess.deliverRawText(error.toXML());
            sess.close();
        }
        
        logger.debug("Clear user groups members and admins");
        String jidAsString = appendDomain(userImName);
        JID jid = new JID(jidAsString);
        
        if (userGroups != null) {
            for (String groupName : userGroups) {
                removeUserFromGroup(groupName, jid);
            }
        }        
        
        logger.debug("Delete user: " + userImName);
        User ofUser = new User(userImName, null, null, null, null);
        UserManager.getInstance().deleteUser(ofUser);        
    }
    
    public static void addUserToGroup(String groupName, JID jid) {
        logger.debug(String.format("Add user %s to group %s", jid.toString(), groupName));
        try {
            if (StringUtils.isNotBlank(groupName)) {
                Group group = GroupManager.getInstance().getGroup(groupName);
                // automatically add the user in group as admin. Openfire holds two collections
                // members collection and admins collection. A user is contained either in members
                // or in admins collection, not in both
                if (group != null) {
                    group.getMembers().add(jid);
                }
            }
        } catch (GroupNotFoundException ex) {
            logger.error(String.format("Add user %s to group %s: Group not found", jid.toString(), groupName));
        }
    }
    
    public static void removeUserFromGroup(String groupName, JID jid) {
        logger.debug(String.format("remove user %s from group %s", jid.toString(), groupName));
        try {
            if (StringUtils.isNotBlank(groupName)) {
                Group group = GroupManager.getInstance().getGroup(groupName);
                if (group != null) {
                    group.getMembers().remove(jid);
                    // it's not supposed to be an admin, but just in case
                    group.getAdmins().remove(jid);
                }
            }
        } catch (GroupNotFoundException ex) {
            logger.error(String.format("Remove user %s from group %s: Group not found", jid.toString(), groupName));
        }
    }    

    public static String appendDomain(String userName) {
        if (userName.indexOf("@") == -1) {
            // No @ in the domain so assume this is our domain.
            return userName + "@" + XMPPServer.getInstance().getServerInfo().getXMPPDomain();
        }

        return userName;
    }
}
