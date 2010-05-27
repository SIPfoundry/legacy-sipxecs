/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.openfire.vcard.provider;

import java.io.StringReader;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.Calendar;

import org.dom4j.Document;
import org.dom4j.DocumentException;
import org.dom4j.Element;
import org.dom4j.io.SAXReader;
import org.jivesoftware.openfire.MessageRouter;
import org.jivesoftware.openfire.PresenceManager;
import org.jivesoftware.openfire.XMPPServer;
import org.jivesoftware.openfire.XMPPServerInfo;
import org.jivesoftware.openfire.handler.PresenceUpdateHandler;
import org.jivesoftware.openfire.user.User;
import org.jivesoftware.openfire.user.UserManager;
import org.jivesoftware.openfire.user.UserNotFoundException;
import org.jivesoftware.openfire.vcard.VCardManager;
import org.jivesoftware.util.Log;
import org.xmpp.packet.Message;
import org.xmpp.packet.Presence;
import org.xmpp.packet.Message.Type;

public class ContactInfoHandlerImp implements ContactInfoHandler {
    public void notifyContactChange(String userName) {
        if (VCardManager.getProvider() instanceof SipXVCardProvider) {
            Log.info("Contact Change Notification received for user " + userName);
            SipXVCardProvider prov = (SipXVCardProvider) VCardManager.getProvider();
            Element vCard = prov.cacheVCard(userName);
            try {
                VCardManager.getInstance().reset();
                updateAvatar(userName, vCard, true);
            } catch (Exception e) {
                Log.error("In ContactInfoHandlerImp set/update VCard failed! " + e.getMessage());
            }
        }
    }

    public static void updateAvatar(String username, Element vCard, boolean notify) {

        if (vCard.element("PHOTO") == null)
            return;

        Element binValElement = vCard.element("PHOTO").element("BINVAL");
        if (binValElement == null)
            return;

        try {

            String avatarStr = binValElement.getText();

            XMPPServer server = XMPPServer.getInstance();
            XMPPServerInfo info = server.getServerInfo();
            String aor = username + "@" + info.getXMPPDomain();
            String itemId = getItemId(avatarStr.getBytes());

            Presence presenceAvatar = createPresenceAvatar(aor, itemId);
            PresenceUpdateHandler puh = server.getPresenceUpdateHandler();
            puh.process(presenceAvatar);

            // Sending announcement to the client
            if (notify) {
                MessageRouter mr = server.getMessageRouter();
                mr.route(createNotificationMessage(info.getXMPPDomain(), aor));
            }

        } catch (Exception ex) {
            Log.error(ex.getMessage());
        }
    }

    public static String getCurrentDate() {
        DateFormat dateFormat = new SimpleDateFormat("yyyy/MM/dd HH:mm:ss");
        Calendar cal = Calendar.getInstance();
        return dateFormat.format(cal.getTime());
    }

    public static String getItemId(byte[] avatarBytes) {
        try {
            MessageDigest md = MessageDigest.getInstance("SHA-1");
            md.update(avatarBytes);
            // md.update( int ) processes only the low order 8-bits. It actually expects an
            // unsigned byte.
            byte[] digest = md.digest();

            return byteArrayToString(digest);
        }

        catch (NoSuchAlgorithmException ex) {
            Log.error("no such algorithm of SHA-1 " + ex.getMessage());
            return "thisisafakeitemid";
        }

        catch (Exception ex) {
            Log.error(ex.getMessage());
            return "thisisafakeitemid";
        }
    }

    public static String byteArrayToString(byte[] bytes) {
        StringBuilder sb = new StringBuilder(bytes.length * 2);
        for (int i = 0; i < bytes.length; i++) {
            sb.append(String.format("%02x", bytes[i]));
        }
        return sb.toString();
    }

    public static Presence createPresenceAvatar(String aor, String itemId) {
        StringBuilder builder = new StringBuilder("<presence>");
        builder.append("<priority>1</priority>");
        builder
                .append("<c xmlns='http://jabber.org/protocol/caps' node='http://pidgin.im/' hash='sha-1' ver='I22W7CegORwdbnu0ZiQwGpxr0Go='/>");
        builder.append("<x xmlns='vcard-temp:x:update'>");
        builder.append("<photo>");
        builder.append(itemId);
        builder.append("</photo></x></presence>");

        String xmlstr = builder.toString();
        SAXReader sreader = new SAXReader();

        try {
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
        } catch (DocumentException e) {
            Log.error("In createPresenceAvatar, DocumentException " + e.getMessage());
            return null;
        } catch (UserNotFoundException e) {
            Log.error("In createPresenceAvatar, UserNotFoundException for aor " + aor);
            return null;
        }

    }

    public static Message createNotificationMessage(String server, String aor) {
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
        builder.append("--- User info is changed from server, please relogin to synchronize ---");
        msg.setBody(builder.toString());

        return msg;
    }
}
