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

import java.util.Collection;

import org.apache.commons.lang.StringUtils;
import org.apache.log4j.Logger;
import org.jivesoftware.openfire.group.Group;
import org.jivesoftware.openfire.group.GroupManager;
import org.jivesoftware.openfire.group.GroupNotFoundException;

public class GroupShared {
    private static Logger logger = Logger.getLogger(GroupShared.class);

    public static void removeGroup(String groupName) {
        try {
            Group imGroup = GroupManager.getInstance().getGroup(groupName);
            if (imGroup != null) {
                logger.debug("deleting group: " + imGroup.getName());
                imGroup.getMembers().clear();
                imGroup.getAdmins().clear();
                GroupManager.getInstance().deleteGroup(imGroup);
            }
        } catch (GroupNotFoundException ex) {
            logger.error(String.format("Delete group: Group %s not found", groupName));
        }
    }
    
    public static void setGroupProperties(Group group) {
        group.getProperties().put("sharedRoster.showInRoster", "onlyGroup");
        group.getProperties().put("sharedRoster.displayName", group.getName());
        Collection<Group> sharedGroups = GroupManager.getInstance().getPublicSharedGroups();
        StringBuilder buf = new StringBuilder();
        if (sharedGroups != null) {           
            String sep = "";
            for (Group sharedGroup : sharedGroups) {
                String groupName = sharedGroup.getName();
                buf.append(sep).append(groupName);
                sep = ",";
            }            
        }
        group.getProperties().put("sharedRoster.groupList", buf.toString());      
    }
    
    public static void setGroupProperties(String groupName) {
        try {
            Group imGroup = GroupManager.getInstance().getGroup(groupName);
            setGroupProperties(imGroup);
        } catch (GroupNotFoundException e) {
            logger.debug("Group not found: " + groupName + " cannot update properties");
        }

    }
}