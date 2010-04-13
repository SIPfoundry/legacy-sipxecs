/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.openfire.config;

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
   public boolean equals(Object other) {
       //check for self-comparison
       if ( this == other ) return true;

       if ( !(other instanceof XmppChatRoom) ) return false;

       //cast to native object is now safe
       XmppChatRoom otherChatRoom = (XmppChatRoom)other;

       //now a proper field-by-field evaluation can be made
       return 
           subdomain.equals(otherChatRoom.subdomain) &&
           roomName.equals(otherChatRoom.roomName) &&
           description.equals(otherChatRoom.description) &&
           password.equals(otherChatRoom.password) &&
           conferenceExtension.equals(otherChatRoom.conferenceExtension) &&
           conferenceName.equals(otherChatRoom.conferenceName) &&
           conferenceReachabilityInfo.equals(otherChatRoom.conferenceReachabilityInfo) &&
           owner.equals(otherChatRoom.owner) &&
           logRoomConversations == otherChatRoom.logRoomConversations &&
           isPublicRoom == otherChatRoom.isPublicRoom &&
           membersOnly == otherChatRoom.membersOnly &&
           isPersistent == otherChatRoom.isPersistent &&
           isModerated == otherChatRoom.isModerated;
   }   

   @Override
   public int hashCode()
   {
       return subdomain.hashCode() * roomName.hashCode();
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
