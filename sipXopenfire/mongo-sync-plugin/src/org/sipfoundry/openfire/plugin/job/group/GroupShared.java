package org.sipfoundry.openfire.plugin.job.group;

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
                GroupManager.getInstance().deleteGroup(imGroup);
            }
        } catch (GroupNotFoundException ex) {
            logger.error(String.format("Delete group: Group %s not found", groupName));
        }
    }
}