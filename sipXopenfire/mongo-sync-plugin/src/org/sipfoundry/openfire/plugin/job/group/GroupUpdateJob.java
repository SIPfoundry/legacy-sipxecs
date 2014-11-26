/**
 *
 *
 * Copyright (c) 2014 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.openfire.plugin.job.group;


import org.apache.commons.lang.StringUtils;
import org.apache.log4j.Logger;
import org.jivesoftware.openfire.group.Group;
import org.jivesoftware.openfire.group.GroupAlreadyExistsException;
import org.jivesoftware.openfire.group.GroupManager;
import org.jivesoftware.openfire.group.GroupNotFoundException;
import org.sipfoundry.openfire.plugin.job.user.UserShared;
import org.sipfoundry.openfire.provider.CacheHolder;
import org.sipfoundry.openfire.sync.job.Job;
import org.xmpp.packet.JID;

public class GroupUpdateJob implements Job {
    private static Logger logger = Logger.getLogger(GroupUpdateJob.class);

    private static final long serialVersionUID = 1L;

    private final String groupName;
    private final String oldGroupName;
    private final boolean isImGroup;
    private final String description;
    private final JID imbot;
    private final boolean isMyBuddyEnabled;
    private final String id;

    public GroupUpdateJob(String id, String groupName, String oldGroupName, boolean imGroup, String description,
            boolean isMybuddyEnabled, JID imbot) {
        this.groupName = groupName;
        this.oldGroupName = oldGroupName;
        this.isImGroup = imGroup;
        this.description = description;
        this.imbot = imbot;
        this.isMyBuddyEnabled = isMybuddyEnabled;
        this.id = id;
    }

    @Override
    public void process() {
        logger.debug("start processing " + toString());

        // not an imgroup but in cache: delete it
        if (!isImGroup) {
            GroupShared.removeGroup(groupName);
            logger.debug("end processing " + toString());
            return;
        }
        
        if (isImGroup && !StringUtils.isBlank(groupName)) {  
            logger.debug("Add mybuddy: " + isMyBuddyEnabled);
            Group group = null;
            try {
                //group update is attempted
                if (!StringUtils.equalsIgnoreCase(oldGroupName, groupName) && !StringUtils.isBlank(oldGroupName)) {
                    group = GroupManager.getInstance().getGroup(oldGroupName);                    
                    //Openfire requires members and admins deletion also for update group operation (maybe a bug)
                    //After group updates, users will be added cleanly to the updated group by GroupUpdateJob
                    group.getMembers().clear();
                    group.getAdmins().clear();

                    //setName method will push group update event in openfire
                    group.setName(groupName);
                }
            } catch (GroupNotFoundException e) {
                logger.debug("Group not found " + groupName, e);
            }
            //A new group is attempted to be created
            if (group == null) {
                try {
                    group = GroupManager.getInstance().getGroup(groupName, true);                    
                } catch (GroupNotFoundException e) {
                    logger.debug("Group already exists " + groupName, e);
                }
            }
                
            if (group == null) {
                logger.debug("Cannot update/create group: " + groupName);
                return;
            }
            
            //keep actual group name in cache
            CacheHolder.putGroup(id, groupName);
            
            // add mybuddy to group (no UserUpdateJob is triggered for mybuddy, as mybuddy is not part of a group in sipxconfig)
            if (isMyBuddyEnabled) {
                group.getMembers().add(imbot);
            } else {
                group.getMembers().remove(imbot);
            }
               
            // apply description in case it was changed
            if (StringUtils.isNotBlank(description)) {
                group.setDescription(description);
            }
            //force cache update with new/changed group
            try {
                group = GroupManager.getInstance().getGroup(groupName, true);
                group.getProperties().put("sharedRoster.displayName", groupName);
                group.getProperties().put("sharedRoster.groupList", "");
            } catch (GroupNotFoundException e) {
                logger.debug("Cannot update group " + groupName, e);
            }
        }

        logger.debug("end processing " + toString());
    }

    @Override
    public String toString() {
        return "GroupUpdateJob [groupName=" + groupName + ", oldGroupName=" + oldGroupName + ", isImGroup="
                + isImGroup + ", description=" + description + "]";
    }

    @Override
    public int hashCode() {
        final int prime = 31;
        int result = 1;
        result = prime * result + ((description == null) ? 0 : description.hashCode());
        result = prime * result + ((groupName == null) ? 0 : groupName.hashCode());
        result = prime * result + (isImGroup ? 1231 : 1237);
        result = prime * result + ((oldGroupName == null) ? 0 : oldGroupName.hashCode());
        return result;
    }

    @Override
    public boolean equals(Object obj) {
        if (this == obj) {
            return true;
        }
        if (obj == null) {
            return false;
        }
        if (getClass() != obj.getClass()) {
            return false;
        }
        GroupUpdateJob other = (GroupUpdateJob) obj;
        if (description == null) {
            if (other.description != null) {
                return false;
            }
        } else if (!description.equals(other.description)) {
            return false;
        }
        if (groupName == null) {
            if (other.groupName != null) {
                return false;
            }
        } else if (!groupName.equals(other.groupName)) {
            return false;
        }
        if (isImGroup != other.isImGroup) {
            return false;
        }
        if (oldGroupName == null) {
            if (other.oldGroupName != null) {
                return false;
            }
        } else if (!oldGroupName.equals(other.oldGroupName)) {
            return false;
        }
        return true;
    }
}
