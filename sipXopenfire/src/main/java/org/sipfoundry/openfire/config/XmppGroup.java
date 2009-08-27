package org.sipfoundry.openfire.config;

import java.util.Collection;
import java.util.HashSet;
import java.util.LinkedList;

public class XmppGroup {
    
    private String groupName;
    
    private String description;
    
    private String administrator;
    
    private HashSet<XmppGroupMember> members = new HashSet<XmppGroupMember>();

    /**
     * @param groupName the groupName to set
     */
    public void setGroupName(String groupName) {
        this.groupName = groupName;
    }

    /**
     * @return the groupName
     */
    public String getGroupName() {
        return groupName;
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
    
    public Collection<XmppGroupMember> getMembers() {
        return this.members;
    }
    
    public void addMember(XmppGroupMember member) {
        this.members.add(member);
    }

    
    /**
     * @param administrator the administrator to set
     */
    public void setAdministrator(String administrator) {
        this.administrator = XmppAccountInfo.appendDomain(administrator);
       
    }

    /**
     * @return the administrator
     */
    public String getAdministrator() {
        return administrator;
    }
    
    

}
