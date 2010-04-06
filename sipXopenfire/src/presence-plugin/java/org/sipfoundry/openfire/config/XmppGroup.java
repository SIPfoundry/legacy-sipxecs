/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.openfire.config;

import java.util.Collection;
import java.util.HashMap;
import java.util.HashSet;
import java.util.LinkedList;

import org.apache.log4j.Logger;

public class XmppGroup extends XmppConfigurationElement {
    private static Logger logger = Logger.getLogger(XmppGroup.class);

    private String groupName = "";
    
    private String description = "";
    
    private String administrator = "";
    
    private HashMap<String,XmppGroupMember> members = new HashMap<String,XmppGroupMember>();
    // NOTE: extend the equals() method if new instance variables get added


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
    
    @Override 
    public boolean equals(Object other) {
        //check for self-comparison
        if ( this == other ) return true;

        if ( !(other instanceof XmppGroup) ) return false;

        //cast to native object is now safe
        XmppGroup otherGroup = (XmppGroup)other;

        //now a proper field-by-field evaluation can be made
        try{
            return groupName.equals( otherGroup.groupName ) &&
                   description.equals( otherGroup.description ) &&
                   administrator.equals( otherGroup.administrator ) &&
                   members.size() == otherGroup.members.size() &&
                   members.values().containsAll(otherGroup.members.values());
        } catch( Exception e ){
            logger.error("Caught: ", e);
            return false;
        }
    }
    
    @Override
    public int hashCode()
    {
        return groupName.hashCode();
    }

    @Override
    public String toString()
    {
        return new StringBuilder("XmppGroup='")
        .append(this.getGroupName())
        .append("'\n    status='")
        .append(this.getStatus())
        .append("'\n    admin='")
        .append(this.getAdministrator())
        .append("'\n    desc='")
        .append(this.getDescription())
        .append("'\n    members='")
        .append(this.getMembers())
        .append("'\n===============\n").toString();     
    }
}
