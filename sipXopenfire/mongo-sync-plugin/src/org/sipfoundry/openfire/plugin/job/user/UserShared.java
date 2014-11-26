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

import org.apache.log4j.Logger;
import org.jivesoftware.openfire.SessionManager;
import org.jivesoftware.openfire.XMPPServer;
import org.jivesoftware.openfire.group.Group;
import org.jivesoftware.openfire.group.GroupManager;
import org.jivesoftware.openfire.session.ClientSession;
import org.jivesoftware.openfire.user.User;
import org.jivesoftware.openfire.user.UserManager;
import org.xmpp.packet.JID;
import org.xmpp.packet.StreamError;

public class UserShared {
    private static Logger logger = Logger.getLogger(UserShared.class);

    public static void removeUser(String userImName) {
        logger.debug("Delete user: " + userImName);
        User ofUser = new User(userImName, null, null, null, null);
        UserManager.getInstance().deleteUser(ofUser);
        
        logger.debug("Clear user groups members and admins");
        String jidAsString = appendDomain(userImName);
        JID jid = new JID(jidAsString);
        Collection<Group> groups = GroupManager.getInstance().getGroups();
        for (Group group : groups) {
            group.getMembers().remove(jid);
            group.getAdmins().remove(jid);
        }
        logger.debug("Close deleted user xmpp session");
        final StreamError error = new StreamError(StreamError.Condition.not_authorized);
        for (ClientSession sess : SessionManager.getInstance().getSessions(userImName)) {
            sess.deliverRawText(error.toXML());
            sess.close();
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
