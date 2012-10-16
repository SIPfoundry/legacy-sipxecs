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
package org.sipfoundry.openfire.vcard.synchserver;

import java.io.StringReader;
import java.security.MessageDigest;
import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.Calendar;

import org.dom4j.Document;
import org.dom4j.Element;
import org.dom4j.io.SAXReader;
import org.jivesoftware.openfire.MessageRouter;
import org.jivesoftware.openfire.PresenceManager;
import org.jivesoftware.openfire.XMPPServer;
import org.jivesoftware.openfire.XMPPServerInfo;
import org.jivesoftware.openfire.handler.PresenceUpdateHandler;
import org.jivesoftware.openfire.user.User;
import org.jivesoftware.openfire.user.UserManager;
import org.xmpp.packet.Message;
import org.xmpp.packet.Presence;
import org.xmpp.packet.Message.Type;

public class Util {
    private static Message createNotificationMessage(String server, String aor) {
        Message msg = new Message();
        msg.setFrom(server);
        msg.setTo(aor);
        msg.setType(Type.headline);
        msg.setSubject("Warning");
        StringBuilder builder = new StringBuilder();
        builder.append("From:");
        builder.append(server);
        builder.append("\n");
        builder.append("To:");
        builder.append(aor);
        builder.append("\n");
        builder.append("Time:");
        builder.append(getCurrentDate());
        builder.append("\n");
        builder.append("Detail: \n");
        builder.append("--- User info has changed on the server; you MUST relogin to synchronize before modifying any of your profile information including avatar ---");
        msg.setBody(builder.toString());

        return msg;
    }

    public static String getCurrentDate() {
        DateFormat dateFormat = new SimpleDateFormat("yyyy/MM/dd HH:mm:ss");
        Calendar cal = Calendar.getInstance();
        return dateFormat.format(cal.getTime());
    }

    public static void updateAvatar(String username, Element vCard) throws Exception{
        if (vCard.element("PHOTO") == null) {
            return;
        }
        Element binValElement = vCard.element("PHOTO").element("BINVAL");
        if (binValElement == null) {
            return;
        }
        String avatarStr = binValElement.getText();
        XMPPServer server = XMPPServer.getInstance();
        XMPPServerInfo info = server.getServerInfo();
        String aor = username + "@" + info.getXMPPDomain();
        String itemId = getItemId(avatarStr.getBytes());
        Presence presenceAvatar = createPresenceAvatar(aor, itemId);
        PresenceUpdateHandler puh = server.getPresenceUpdateHandler();
        puh.process(presenceAvatar);
    }

    private static String getItemId(byte[] avatarBytes) throws Exception {
        MessageDigest md = MessageDigest.getInstance("SHA-1");
        md.update(avatarBytes);
        byte[] digest = md.digest();
        return byteArrayToString(digest);
    }

    private static String byteArrayToString(byte[] bytes) {
        StringBuilder sb = new StringBuilder(bytes.length * 2);
        for (int i = 0; i < bytes.length; i++) {
            sb.append(String.format("%02x", bytes[i]));
        }
        return sb.toString();
    }

    private static Presence createPresenceAvatar(String aor, String itemId) throws Exception {
        StringBuilder builder = new StringBuilder("<presence>");
        builder.append("<priority>1</priority>");
        builder.append("<c xmlns='http://jabber.org/protocol/caps' node='http://pidgin.im/' hash='sha-1' ver='I22W7CegORwdbnu0ZiQwGpxr0Go='/>");
        builder.append("<x xmlns='vcard-temp:x:update'>");
        builder.append("<photo>");
        builder.append(itemId);
        builder.append("</photo></x></presence>");

        String xmlstr = builder.toString();
        SAXReader sreader = new SAXReader();

        Document avatarDoc = sreader.read(new StringReader(xmlstr));
        Element rootElement = avatarDoc.getRootElement();

        Presence avatar = new Presence(rootElement);

        avatar.setFrom(aor);

        UserManager um = XMPPServer.getInstance().getUserManager();
        User me = um.getUser(aor);

        PresenceManager pm = XMPPServer.getInstance().getPresenceManager();
        Presence mypresence = pm.getPresence(me);

        avatar.setType(mypresence.getType());
        avatar.setShow(mypresence.getShow());
        avatar.setStatus(mypresence.getStatus());
        avatar.setID(aor + "_presenceAvatar");
        return avatar;
    }

    public static void notify(String username) {
        XMPPServer server = XMPPServer.getInstance();
        XMPPServerInfo info = server.getServerInfo();
        String aor = username + "@" + info.getXMPPDomain();
        MessageRouter mr = server.getMessageRouter();
        mr.route(createNotificationMessage(info.getXMPPDomain(), aor));
    }
}
