/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.openfire.config;

import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

import org.apache.log4j.Logger;
import org.jivesoftware.openfire.group.Group;
import org.jivesoftware.openfire.group.GroupNotFoundException;
import org.sipfoundry.openfire.plugin.presence.SipXOpenfirePlugin;
import org.xmpp.packet.JID;

public class XmppAccountInfo {

    private static final Logger logger = Logger.getLogger(XmppAccountInfo.class);

    private Map<String, XmppUserAccount> userAccounts = new HashMap<String, XmppUserAccount>();

    private Map<String, XmppGroup> groups = new HashMap<String, XmppGroup>();

    private Map<String, XmppChatRoom> chatRooms = new HashMap<String, XmppChatRoom>();

    public static String appendDomain(String userName) {
        if (userName.indexOf("@") == -1) {
            // No @ in the domain so assume this is our domain.
            return userName + "@" + SipXOpenfirePlugin.getInstance().getXmppDomain();
        } else {
            return userName;         
        }
    }

    public XmppAccountInfo() {

    }

    public void addAccount(XmppUserAccount xmppUserAccount) {
        this.userAccounts.put(xmppUserAccount.getUserName(), xmppUserAccount);
    }

    public void addGroup(XmppGroup group) {
        this.groups.put(group.getGroupName(), group);
    }

    public void addChatRoom(XmppChatRoom xmppChatRoom) {
        this.chatRooms.put(xmppChatRoom.getSubdomain() + ":" + xmppChatRoom.getRoomName(), xmppChatRoom);
    }

    // user account getters
    public Map<String, XmppUserAccount> getXmppUserAccountMap() {
        return this.userAccounts;
    }

    public XmppUserAccount getXmppUserAccount(String name) {
        return this.userAccounts.get(name);
    }

    public Collection<XmppUserAccount> getXmppUserAccounts() {
        return this.userAccounts.values();
    }

    public Set<String> getXmppUserAccountNames() {
        return this.userAccounts.keySet();
    }

    // group getters
    public Map<String, XmppGroup> getXmppGroupMap() {
        return this.groups;
    }

    public XmppGroup getXmppGroup(String name) {
        return this.groups.get(name);
    }

    public Collection<XmppGroup> getXmppGroups() {
        return this.groups.values();
    }

    public Set<String> getXmppGroupNames    () {
        return this.groups.keySet();
    }

    // chatroom getters
    public Map<String, XmppChatRoom> getXmppChatRoomMap() {
        return this.chatRooms;
    }

    public XmppChatRoom getXmppChatRoom(String subdomain, String name) {
        String key = subdomain + ":" + name;
        return this.chatRooms.get(key);
    }

    public Collection<XmppChatRoom> getXmppChatRooms() {
        return this.chatRooms.values();
    }

    public Set<String> getXmppChatRoomNames() {
        return this.chatRooms.keySet();
    }

    // universal getter
    public Collection<XmppConfigurationElement> getAllElements() {
        Collection<XmppConfigurationElement> allElements = new ArrayList<XmppConfigurationElement>();
        allElements.addAll(getXmppUserAccounts());
        allElements.addAll(getXmppGroups());
        allElements.addAll(getXmppChatRooms());
        return allElements;
    }
}
