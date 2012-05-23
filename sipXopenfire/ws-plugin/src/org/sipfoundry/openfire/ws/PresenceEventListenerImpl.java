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

import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.io.InputStream;

import org.apache.commons.httpclient.HttpClient;
import org.apache.commons.httpclient.methods.InputStreamRequestEntity;
import org.apache.commons.httpclient.methods.PostMethod;
import org.apache.commons.httpclient.methods.RequestEntity;
import org.apache.commons.io.IOUtils;
import org.apache.commons.lang.StringUtils;
import org.jivesoftware.openfire.session.ClientSession;
import org.jivesoftware.openfire.user.PresenceEventListener;
import org.sipfoundry.commons.userdb.User;
import org.sipfoundry.commons.util.UnfortunateLackOfSpringSupportFactory;
import org.xmpp.packet.JID;
import org.xmpp.packet.Presence;

public class PresenceEventListenerImpl implements PresenceEventListener {

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
            User user = UnfortunateLackOfSpringSupportFactory.getValidUsers().getUserByJid(presence.getFrom().getNode());
            if (user != null) {
                invokePost(user.getUserName(), message);
            }
        } catch (Exception e) {
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

    private String getRestServerUrl(String fqdn, int port) {
        return String.format("https://%s:%d/receiver", fqdn, port);
    }

    private String invokePost(String userId, String message) throws Exception {
        String response = null;
        InputStream stream = null;
        String websocketAddress = System.getProperty("websocket.address");
        String websocketPort = System.getProperty("websocket.port");
        if (websocketAddress != null && websocketPort != null) {
            PostMethod method = new PostMethod(getRestServerUrl(websocketAddress, new Integer(websocketPort)));
            RequestEntity re = new InputStreamRequestEntity(new ByteArrayInputStream(message.getBytes()), "text/x-json");
            method.setRequestEntity(re);
            method.addRequestHeader("user_id", userId);
            try {
                HttpClient client = new HttpClient();
                client.executeMethod(method);
            } catch (IOException e) {
                throw new RuntimeException(e);
            } finally {
                if (method != null) {
                    method.releaseConnection();
                }
                IOUtils.closeQuietly(stream);
            }
        }
        return response;
    }
}
