package org.sipfoundry.openfire.config;

import java.util.Collection;
import java.util.HashMap;
import java.util.HashSet;
import java.util.LinkedList;

import org.apache.log4j.Logger;

public class XmppGroup {
    private static Logger logger = Logger.getLogger(XmppGroup.class);

    private String groupName;
    
    private String description;
    
    private String administrator;
    
    private HashMap<String,XmppGroupMember> members = new HashMap<String,XmppGroupMember>();

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
        return this.members.values();
    }
    
    public void addMember(XmppGroupMember member) {        
        this.members.put(member.getJid(),member);
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

    public boolean hasMember(String jid) {
        return this.members.containsKey(jid);
    }
    
    

}
