package org.sipfoundry.openfire.config;

public class XmppChatRoom {
    private String subdomain;
    private String roomName;
    private String description; 
    private String password;
    private String conferenceExtension;
    private String owner;
    private boolean logRoomConversations;
    private boolean isPublicRoom = true;
    private boolean membersOnly;
    private boolean isPersistant;
    private boolean isRoomListed = true;
    
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
        this.roomName = roomName;
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
     * @param password the password to set
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
   
   public void setPersistant(String flag) {
       this.isPersistant = Boolean.parseBoolean(flag);
   }
   
   public boolean isPersistant() {
       return this.isPersistant;
   }
   
   public void setRoomListed(String flag) {
       this.isRoomListed = Boolean.parseBoolean(flag);
   }
   
   public boolean isRoomListed() {
       return this.isRoomListed;
   }
 
    
}
