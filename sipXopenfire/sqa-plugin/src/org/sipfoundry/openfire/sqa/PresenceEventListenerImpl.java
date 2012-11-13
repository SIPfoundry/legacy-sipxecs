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
package org.sipfoundry.openfire.sqa;

import java.util.Map;

import org.jivesoftware.openfire.XMPPServer;
import org.jivesoftware.openfire.session.ClientSession;
import org.jivesoftware.openfire.user.PresenceEventListener;
import org.jivesoftware.openfire.user.UserNotFoundException;
import org.sipfoundry.commons.userdb.User;
import org.sipfoundry.commons.userdb.ValidUsers;
import org.sipfoundry.commons.util.UnfortunateLackOfSpringSupportFactory;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.xmpp.packet.JID;
import org.xmpp.packet.Presence;

public class PresenceEventListenerImpl implements PresenceEventListener {
    Map<String, SipPresenceBean> m_presenceCache = null;
    ValidUsers m_users = UnfortunateLackOfSpringSupportFactory.getValidUsers();

    private static final Logger logger = LoggerFactory.getLogger(PresenceEventListenerImpl.class);

    public PresenceEventListenerImpl (Map<String, SipPresenceBean> presenceCache) {
        m_presenceCache = presenceCache;
    }
    @Override
    public void availableSession(ClientSession arg0, Presence arg1) {

    }

    @Override
    public void presenceChanged(ClientSession arg0, Presence presence) {
        String userJid = presence.getFrom().getNode();
        SipPresenceBean previousPresenceBean = m_presenceCache.get(userJid);
        //cache is not cleared, but the presence is changed - change status accordingly keeping the sip state
        if(previousPresenceBean != null) {
            User user = m_users.getUserByJid(userJid);
            String callingPartyId = previousPresenceBean.getCallingPartiId();
            User callingParty = m_users.getUser(callingPartyId);
            //Presence changed - save new presence and broadcast -on the phone- to roster
            m_presenceCache.put(userJid, new SipPresenceBean(presence.getStatus(), callingPartyId));
            String presenceMessage = Utils.generateXmppStatusMessageWithSipState(user, callingParty, presence, callingPartyId);
            org.jivesoftware.openfire.user.User ofObserverUser = null;
            try {
                ofObserverUser = XMPPServer.getInstance().getUserManager().getUser(userJid);
                presence.setStatus(presenceMessage);
                ofObserverUser.getRoster().broadcastPresence(presence);
            } catch (UserNotFoundException e) {
                logger.debug("Cannot update user presence ", e);
            }

        }
    }

    @Override
    public void subscribedToPresence(JID arg0, JID arg1) {

    }

    @Override
    public void unavailableSession(ClientSession arg0, Presence arg1) {

    }

    @Override
    public void unsubscribedToPresence(JID arg0, JID arg1) {

    }

}
