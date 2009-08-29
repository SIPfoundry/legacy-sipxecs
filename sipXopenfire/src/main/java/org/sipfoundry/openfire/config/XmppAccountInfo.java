package org.sipfoundry.openfire.config;

import java.util.Collection;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;

import org.apache.log4j.Logger;
import org.jivesoftware.openfire.group.GroupNotFoundException;
import org.sipfoundry.openfire.plugin.presence.SipXOpenfirePlugin;
import org.sipfoundry.openfire.plugin.presence.SipXOpenfirePluginException;

public class XmppAccountInfo {

    private static final Logger logger = Logger.getLogger(XmppAccountInfo.class);
    private static SipXOpenfirePlugin plugin = SipXOpenfirePlugin.getInstance();

    private Map<String,XmppUserAccount> userAccounts = new HashMap<String,XmppUserAccount>();

    private Map<String,XmppGroup> groups = new HashMap<String,XmppGroup>();

    private Map<String,XmppChatRoom> chatRooms = new HashMap<String,XmppChatRoom>();

    protected static String appendDomain(String userName) {
        if (userName.indexOf("@") == -1) {
            // No @ in the domain so assume this is our domain.
            return userName + "@" + SipXOpenfirePlugin.getInstance().getXmppDomain();
        } else {
            return userName;
        }
    }

    public XmppAccountInfo() {

    }

    public void addAccount(XmppUserAccount xmppUserAccount) throws Exception {
        logger.debug("createUserAccount " + xmppUserAccount.getUserName() +
                " password = " + xmppUserAccount.getPassword());
        plugin.createUserAccount(xmppUserAccount.getUserName(), xmppUserAccount.getPassword(),
                xmppUserAccount.getDisplayName(), xmppUserAccount.getEmail());
        String jid = appendDomain(xmppUserAccount.getUserName());
        String sipUserName = xmppUserAccount.getSipUserName();
        logger.debug("setSipId " + jid + " sipUserName " + sipUserName);
        plugin.setSipId(jid, sipUserName);
        this.userAccounts.put(xmppUserAccount.getUserName(),xmppUserAccount);
        
    }

    public void addGroup(XmppGroup group) throws Exception {
        logger.info("createGroup " + group.getGroupName() + " description "
                + group.getDescription());
        plugin
                .createGroup(group.getGroupName(), group.getAdministrator(), group
                        .getDescription());
        for (XmppGroupMember member : group.getMembers()) {
            String userJid = appendDomain(member.getUserName());
            SipXOpenfirePlugin.getInstance().addUserToGroup(userJid, group.getGroupName());
        }
        this.groups.put(group.getGroupName(),group);
        
    }

    public void addChatRoom(XmppChatRoom xmppChatRoom) throws Exception {
        boolean moderated = false;
        boolean allowInvite = false;
       logger.info(String.format("createChatRoom %s\n %s\n %s\n %s",
                xmppChatRoom.getSubdomain(), xmppChatRoom.getRoomName(), xmppChatRoom
                        .getDescription(), xmppChatRoom.getConferenceExtension()));
        plugin.createChatRoom(xmppChatRoom.getSubdomain(),
                xmppChatRoom.getOwner(),
                xmppChatRoom.getRoomName(), 
                xmppChatRoom.isRoomListed(),
                moderated,
                xmppChatRoom.isMembersOnly(),
                allowInvite,
                xmppChatRoom.isPublicRoom(), 
                xmppChatRoom.isLogRoomConversations(), 
                xmppChatRoom.isPersistant(),
                xmppChatRoom.getPassword(), 
                xmppChatRoom.getDescription(),
                xmppChatRoom.getConferenceExtension());
        this.chatRooms.put(xmppChatRoom.getSubdomain() + ":" + 
                xmppChatRoom.getRoomName(), xmppChatRoom);
    
    }
    
    public XmppUserAccount getXmppUserAccount(String name) {
        return this.userAccounts.get(name);
    }
    public XmppGroup getXmppGroup(String name) {
        return this.groups.get(name);
    }
    
    public XmppChatRoom getXmppChatRoom(String subdomain, String name) {
        String key = subdomain + ":" + name;
        return this.chatRooms.get(key);
    }

    public Collection<XmppChatRoom> getXmppChatRooms() {
       return this.chatRooms.values();
    }

}
