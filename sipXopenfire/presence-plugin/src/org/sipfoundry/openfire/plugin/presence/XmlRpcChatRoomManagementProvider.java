/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.openfire.plugin.presence;

import java.util.Collection;
import java.util.Map;

import org.apache.log4j.Logger;
import org.xmpp.packet.JID;

public class XmlRpcChatRoomManagementProvider extends XmlRpcProvider {

    private static Logger log = Logger.getLogger(XmlRpcChatRoomManagementProvider.class);
    public final static String SERVICE_PATH = "/plugins/sipx-openfire-presence/chatroom";
    public final static String SERVICE_NAME = "chatroom";
    public final static String SERVER = "chatRoomManagementServer";


    /**
     * Get the chat room members.
     *
     * @param subdomain -- server subdomain.
     * @param roomName -- room name.
     * @param userName -- userName
     *
     */
    public Map<String, Object> getMembers(String subdomain, String roomName) {
        try {
            Collection<JID> members = getPlugin().getMembers(subdomain,roomName);
            Map<String, Object> retval = createSuccessMap();
            retval.put(ROOM_MEMBERS, members.toArray());
            return retval;
        } catch ( Exception ex) {

            return createErrorMap(ErrorCode.GET_CHAT_ROOM_MEMBERS,ex.getMessage());

        }
    }





    public Map<String, Object> kickMember(String subdomain, String roomName, String password, String member,
            String reason) {
        try {
            Map<String, Object> retval = createSuccessMap();
            String memberJid = appendDomain(member);
            getPlugin().kickOccupant(subdomain, roomName, password, memberJid, reason);
            return retval;
        } catch (Exception ex) {
            return createErrorMap(ErrorCode.GET_CHAT_ROOM_ATTRIBUTES, ex.getMessage());
        }
    }

    public Map<String, Object> inviteOccupant(String subdomain, String roomName,
            String member, String password, String reason ) {
        try {
            Map<String, Object> retval = createSuccessMap();
            String memberJid = appendDomain(member);
            getPlugin().inviteOccupant(subdomain, roomName, memberJid, password, reason);
            return retval;
        }catch (Exception ex) {
            log.error("Exception occured during processing", ex);
            return createErrorMap(ErrorCode.GET_CHAT_ROOM_ATTRIBUTES, ex.getMessage());
        }
    }



}
