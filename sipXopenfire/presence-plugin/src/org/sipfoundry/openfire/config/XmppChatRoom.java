/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.openfire.config;

import org.jivesoftware.openfire.muc.ConflictException;
import org.jivesoftware.openfire.muc.ForbiddenException;
import org.jivesoftware.openfire.muc.NotAllowedException;
import org.sipfoundry.openfire.plugin.presence.SipXOpenfirePlugin;

public class XmppChatRoom extends XmppConfigurationElement{
    private String subdomain = "";
    private String roomName = "";
    private String description = "";
    private String password = "";
    private String conferenceExtension = "";
    private String conferenceName = "";
    private String conferenceReachabilityInfo = "";
    private String owner = "";
    private boolean logRoomConversations = false;
    private boolean isPublicRoom = true;
    private boolean membersOnly = false;
    private boolean isPersistent = false;
    private boolean isModerated = false;
    // NOTE: extend the equals() method if new instance variables get added

    public XmppChatRoom() {

    }

    /**
     * @param subdomain the subdomain to set
     */
    public void setSubdomain(String subdomain) {
        this.subdomain = subdomain;
    }
    /**
     * @return the subdomain
     */
    public String getSubdomain() {
        return subdomain;
    }
    /**
     * @param roomName the roomName to set
     */
    public void setRoomName(String roomName) {
        setConferenceName(roomName);         // Conference Name needs to preserve capitalization for proper SIP routing
        this.roomName = roomName.toLowerCase(); // MUC room needs to be lowercase to avoid confusing openfire
    }
    /**
     * @return the roomName
     */
    public String getRoomName() {
        return roomName;
    }
    /**
     * @param description the description to set
     */
    public void setDescription(String description) {
        this.description = description;
    }
    /**
     * @return the description
     */
    public String getDescription() {
        return description;
    }
    /**
     * @param password - the password to set for the IM chatroom and the
     *                   the one required to enter the associated audio conference
     */
    public void setPassword(String password) {
        this.password = password;
    }
    /**
     * @return the password
     */
    public String getPassword() {
        return password;
    }

    /**
     * @param conferenceExtension the conferenceExtension to set
     */
    public void setConferenceExtension(String conferenceExtension) {
        this.conferenceExtension = conferenceExtension;
    }

    /**
     * @return the conferenceExtension
     */
    public String getConferenceExtension() {
        return conferenceExtension;
    }

    /**
     * @param conferenceName the conferenceName to set
     */
    public void setConferenceName(String conferenceName) {
        this.conferenceName = conferenceName;
    }

    /**
     * @return the conferenceName
     */
    public String getConferenceName() {
        return conferenceName;
    }

    public String getConferenceReachabilityInfo() {
        return conferenceReachabilityInfo;
    }

    public void setConferenceReachabilityInfo( String conferenceReachabilityInfo ) {
        this.conferenceReachabilityInfo = conferenceReachabilityInfo;
    }

    public String getOwner() {
        return owner;
    }

    public void setOwner(String owner) throws Exception {
        if ( owner.indexOf("@") != -1) {
            throw new IllegalArgumentException("Argument must not include domain name");
        }
        this.owner = XmppAccountInfo.appendDomain(owner);
    }


    public void setLogRoomConversations(String flag) throws IllegalArgumentException {
        this.logRoomConversations = Boolean.parseBoolean(flag);
    }

    public boolean isLogRoomConversations() {
        return this.logRoomConversations;
    }

    public void setIsPublicRoom(String flag ) {
        this.isPublicRoom = Boolean.parseBoolean(flag);
    }

    public boolean isPublicRoom() {
        return this.isPublicRoom;
    }

    public void setMembersOnly(String flag) {
        this.membersOnly = Boolean.parseBoolean(flag);
    }

    public boolean isMembersOnly() {
        return this.membersOnly;
    }

    public void setPersistent(String flag) {
        this.isPersistent = Boolean.parseBoolean(flag);
    }

    public boolean isPersistent() {
        return this.isPersistent;
    }

    public void setModerated(String flag) {
        this.isModerated = Boolean.parseBoolean(flag);
    }

    public boolean isModerated() {
        return this.isModerated;
    }

    @Override
    public void update() throws NotAllowedException, ForbiddenException, ConflictException {
        SipXOpenfirePlugin.getInstance().update(this);
    }




    @Override
    public int hashCode() {
        final int prime = 31;
        int result = 1;
        result = prime * result + ((conferenceExtension == null) ? 0 : conferenceExtension.hashCode());
        result = prime * result + ((conferenceName == null) ? 0 : conferenceName.hashCode());
        result = prime * result + ((conferenceReachabilityInfo == null) ? 0 : conferenceReachabilityInfo.hashCode());
        result = prime * result + ((description == null) ? 0 : description.hashCode());
        result = prime * result + (isModerated ? 1231 : 1237);
        result = prime * result + (isPersistent ? 1231 : 1237);
        result = prime * result + (isPublicRoom ? 1231 : 1237);
        result = prime * result + (logRoomConversations ? 1231 : 1237);
        result = prime * result + (membersOnly ? 1231 : 1237);
        result = prime * result + ((owner == null) ? 0 : owner.hashCode());
        result = prime * result + ((password == null) ? 0 : password.hashCode());
        result = prime * result + ((roomName == null) ? 0 : roomName.hashCode());
        result = prime * result + ((subdomain == null) ? 0 : subdomain.hashCode());
        return result;
    }

    @Override
    public boolean equals(Object obj) {
        if (this == obj)
            return true;
        if (obj == null)
            return false;
        if (getClass() != obj.getClass())
            return false;
        XmppChatRoom other = (XmppChatRoom) obj;
        if (conferenceExtension == null) {
            if (other.conferenceExtension != null)
                return false;
        } else if (!conferenceExtension.equals(other.conferenceExtension))
            return false;
        if (conferenceName == null) {
            if (other.conferenceName != null)
                return false;
        } else if (!conferenceName.equals(other.conferenceName))
            return false;
        if (conferenceReachabilityInfo == null) {
            if (other.conferenceReachabilityInfo != null)
                return false;
        } else if (!conferenceReachabilityInfo.equals(other.conferenceReachabilityInfo))
            return false;
        if (description == null) {
            if (other.description != null)
                return false;
        } else if (!description.equals(other.description))
            return false;
        if (isModerated != other.isModerated)
            return false;
        if (isPersistent != other.isPersistent)
            return false;
        if (isPublicRoom != other.isPublicRoom)
            return false;
        if (logRoomConversations != other.logRoomConversations)
            return false;
        if (membersOnly != other.membersOnly)
            return false;
        if (owner == null) {
            if (other.owner != null)
                return false;
        } else if (!owner.equals(other.owner))
            return false;
        if (password == null) {
            if (other.password != null)
                return false;
        } else if (!password.equals(other.password))
            return false;
        if (roomName == null) {
            if (other.roomName != null)
                return false;
        } else if (!roomName.equals(other.roomName))
            return false;
        if (subdomain == null) {
            if (other.subdomain != null)
                return false;
        } else if (!subdomain.equals(other.subdomain))
            return false;
        return true;
    }

    @Override
    public String toString()
    {
        return new StringBuilder("XmppChatRoom='")
        .append("'\n    name='")
        .append(this.getRoomName())
        .append("'\n    subdomain='")
        .append(this.getSubdomain())
        .append("'\n    status='")
        .append(this.getStatus())
        .append("'\n    conf.ext='")
        .append(this.getConferenceExtension())
        .append("'\n    conf.name='")
        .append(this.getConferenceName())
        .append("'\n    conf.reach.info='")
        .append(this.getConferenceReachabilityInfo())
        .append("'\n    desc='")
        .append(this.getDescription())
        .append("'\n    owner='")
        .append(this.getOwner())
        .append("'\n    password='")
        .append(this.getPassword())
        .append("'\n    log.conv='")
        .append(this.isLogRoomConversations())
        .append("'\n    membersOnly='")
        .append(this.isMembersOnly())
        .append("'\n    moderated='")
        .append(this.isModerated())
        .append("'\n    persistent='")
        .append(this.isPersistent())
        .append("'\n    public='")
        .append(this.isPublicRoom())
        .append("'\n===============\n").toString();
    }
}
