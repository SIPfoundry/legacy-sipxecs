/**
 *
 *
 * Copyright (c) 2010 / 2011 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.openfire.ws;

import java.io.IOException;
import java.util.HashMap;
import java.util.Map;

import org.apache.commons.lang.StringUtils;
import org.eclipse.jetty.websocket.WebSocket.Connection;
import org.jivesoftware.openfire.session.ClientSession;
import org.jivesoftware.openfire.user.PresenceEventListener;
import org.xmpp.packet.JID;
import org.xmpp.packet.Presence;

public class PresenceEventListenerImpl implements PresenceEventListener {
	public static final String BEAN_NAME = "presenceEventListener";
	private Map<String, PresenceWebSocket> m_registeredClients = new HashMap<String, PresenceWebSocket>();

	public boolean isUserRegistered(String userId) {
		return m_registeredClients.containsKey(userId);
	}

	public void registerClient(PresenceWebSocket socket) {
		m_registeredClients.put(socket.getUserId(), socket);
	}

	public void unregisterClient(PresenceWebSocket socket) {
		m_registeredClients.remove(socket.getUserId());
	}

	@Override
	public void availableSession(ClientSession arg0, Presence presence) {
		sendPresenceMessage(presence);
	}

	@Override
	public void presenceChanged(ClientSession arg0, Presence presence) {
		sendPresenceMessage(presence);
	}

	private void sendPresenceMessage(Presence presence) {
		try {
			String status = presence.getStatus();
			String message = StringUtils.isEmpty(status) ? "Offline" : status;
			JID from = presence.getFrom();
			PresenceWebSocket socket = m_registeredClients.get(from.getNode());
			if (socket != null) {
				Connection conn = socket.getConnection();
				conn.sendMessage(message);
			}
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}

	@Override
	public void subscribedToPresence(JID arg0, JID arg1) {
	}

	@Override
	public void unavailableSession(ClientSession arg0, Presence presence) {
		sendPresenceMessage(presence);

	}

	@Override
	public void unsubscribedToPresence(JID arg0, JID arg1) {
	}

}
