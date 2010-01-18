package org.sipfoundry.openfire.config;

import java.util.Collection;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import org.apache.log4j.Logger;
import org.jivesoftware.openfire.group.Group;
import org.jivesoftware.openfire.group.GroupNotFoundException;
import org.sipfoundry.openfire.plugin.presence.SipXOpenfirePlugin;
import org.xmpp.packet.JID;

public class XmppAccountInfo {

    private static final Logger logger = Logger.getLogger(XmppAccountInfo.class);
    private static SipXOpenfirePlugin plugin = SipXOpenfirePlugin.getInstance();

    private Map<String, XmppUserAccount> userAccounts = new HashMap<String, XmppUserAccount>();

    private Map<String, XmppGroup> groups = new HashMap<String, XmppGroup>();

    private Map<String, XmppChatRoom> chatRooms = new HashMap<String, XmppChatRoom>();

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
        logger.debug("createUserAccount " + xmppUserAccount.getUserName() + " password = "
                + xmppUserAccount.getPassword());
        plugin.createUserAccount(xmppUserAccount.getUserName(), xmppUserAccount.getPassword(),
                xmppUserAccount.getDisplayName(), xmppUserAccount.getEmail(),
                xmppUserAccount.getAdvertiseOnCallPreference(), xmppUserAccount.getShowOnCallDetailsPreference());
        String jid = appendDomain(xmppUserAccount.getUserName());
        String sipUserName = xmppUserAccount.getSipUserName();
        logger.debug("setSipId " + jid + " sipUserName " + sipUserName);
        plugin.setSipId(jid, sipUserName);
        plugin.setOnThePhoneMessage(sipUserName, xmppUserAccount.getOnThePhoneMessage());
        this.userAccounts.put(xmppUserAccount.getUserName(), xmppUserAccount);
        // Make sure that user can create multi-user chatrooms
        plugin.setAllowedUserForChatServices(jid);
    }

    public void addGroup(XmppGroup group) throws Exception {
        logger.info("createGroup " + group.getGroupName() + " description "
                + group.getDescription());
        if (group.getMembers().isEmpty()) {
            logger.info("no users defined -- not adding group ");
            return;
        }
        boolean isAllAdminGroup = false;
        String adminJid = null;
        JID adminJID = null;
        if (group.getAdministrator() == null) {
            isAllAdminGroup = true;
        } else {
            adminJid = appendDomain(group.getAdministrator());
            adminJID = new JID(adminJid);

        }

        // check if group already exists in openfire
        try{
            Group openfireGroup = plugin.getGroupByName(group.getGroupName());

            // group already exists, make sure that it contains the correct set of members.
            // This is achieved in two operations:
            //  1- Add all members that are currently not in the group but are found in the group from configuration file
            //  2- Remove all members that are currently in the group but not found in the group from configuration file
            logger.info("XmppAccountInfo::addGroup: " + group.getGroupName() + " already exists - enforce members list");
            Collection<String> currentGroupMembers = new HashSet<String>();
            for( JID jid : openfireGroup.getMembers() ){
                currentGroupMembers.add(jid.toBareJID());
            }
            
            Collection<String> desiredGroupMembers = new HashSet<String>();
            Collection<String> desiredGroupMembersBackup = new HashSet<String>();
            for( XmppGroupMember member :group.getMembers() ){
                desiredGroupMembers.add(member.getJid());
                desiredGroupMembersBackup.add(member.getJid());
            }

            //  1- Add all members that are currently not in the group but are found in the group from configuration file
            desiredGroupMembers.removeAll(currentGroupMembers);
            logger.info("Need to add the following members to group '" + group.getGroupName() + "': " + desiredGroupMembers);
            for( String jid : desiredGroupMembers){
                SipXOpenfirePlugin.getInstance().addUserToGroup(jid, group.getGroupName(), isAllAdminGroup);
            }

            //  2- Remove all members that are currently in the group but not found in the group from configuration file
            currentGroupMembers.removeAll(desiredGroupMembersBackup);
            logger.info("Need to remove the following members to group '" + group.getGroupName() + "': " + currentGroupMembers);
            for( String jid : currentGroupMembers){
                SipXOpenfirePlugin.getInstance().removeUserFromGroup(jid, group.getGroupName());
            }
        }
        catch( GroupNotFoundException ex ){
            logger.info("XmppAccountInfo::addGroup: " + group.getGroupName() + " does not exist - create it");
            plugin.createGroup(group.getGroupName(), adminJID, group.getDescription());

            for (XmppGroupMember member : group.getMembers()) {
                String userJid = appendDomain(member.getJid());
                SipXOpenfirePlugin.getInstance().addUserToGroup(userJid, group.getGroupName(), isAllAdminGroup);
            }
        }
        this.groups.put(group.getGroupName(), group);       
    }

    public void addChatRoom(XmppChatRoom xmppChatRoom) throws Exception {
        boolean allowInvite = false;
        logger.info(String.format("createChatRoom %s\n %s\n %s\n %s",
                xmppChatRoom.getSubdomain(), xmppChatRoom.getRoomName(), xmppChatRoom
                        .getDescription(), xmppChatRoom.getConferenceExtension()));
        if ( xmppChatRoom.getSubdomain() == null || xmppChatRoom.getRoomName() == null ) {
            logger.error("Null parameters specified.");
            return;
        }
        plugin.createChatRoom(xmppChatRoom.getSubdomain(), xmppChatRoom.getOwner(), xmppChatRoom
                .getRoomName(), xmppChatRoom.isModerated(), xmppChatRoom
                .isMembersOnly(), allowInvite, xmppChatRoom.isPublicRoom(), xmppChatRoom
                .isLogRoomConversations(), xmppChatRoom.isPersistent(), xmppChatRoom
                .getPassword(), xmppChatRoom.getDescription(), xmppChatRoom
                .getConferenceExtension(), xmppChatRoom.getConferenceName(),
                xmppChatRoom.getConferenceReachabilityInfo());
        this.chatRooms.put(xmppChatRoom.getSubdomain() + ":" + xmppChatRoom.getRoomName(),
                xmppChatRoom);

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
