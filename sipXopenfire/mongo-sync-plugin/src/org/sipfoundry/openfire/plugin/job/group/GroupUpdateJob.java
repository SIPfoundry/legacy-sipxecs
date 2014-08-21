package org.sipfoundry.openfire.plugin.job.group;

import java.util.Collection;
import java.util.List;

import org.apache.commons.lang.StringUtils;
import org.apache.log4j.Logger;
import org.jivesoftware.openfire.group.Group;
import org.jivesoftware.openfire.group.GroupManager;
import org.jivesoftware.openfire.group.GroupNotFoundException;
import org.sipfoundry.openfire.plugin.job.user.UserUpdateJob;
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
    private final List<JID> jidsToAdd;

    public GroupUpdateJob(String groupName, String oldGroupName, boolean imGroup, String description,
            boolean isMybuddyEnabled, JID imbot, List<JID> jidsToAdd) {
        this.groupName = groupName;
        this.oldGroupName = oldGroupName;
        this.isImGroup = imGroup;
        this.description = description;
        this.imbot = imbot;
        this.isMyBuddyEnabled = isMybuddyEnabled;
        this.jidsToAdd = jidsToAdd;
    }

    @Override
    public void process() {
        logger.debug("start processing " + toString());

        // not an imgroup but in cache: delete it
        if (!isImGroup) {
            GroupShared.removeGroup(oldGroupName);
            logger.debug("end processing " + toString());
            return;
        }

        // imgroup and in cache: update it
        if (isImGroup && !StringUtils.isBlank(groupName)) {
            try {
                // clean up mybuddy
                logger.debug("delete mybuddy from group " + oldGroupName);
                Group group = GroupManager.getInstance().getGroup(oldGroupName);
                group.getMembers().remove(imbot);

                // remove old group name from cache if name changed
                if (!StringUtils.equalsIgnoreCase(oldGroupName, groupName)) {
                    GroupShared.removeGroup(oldGroupName);
                    // reload new group from mongo
                    group = GroupManager.getInstance().getGroup(groupName);
                }

                // apply description in case it was changed
                if (StringUtils.isNotBlank(description)) {
                    group.setDescription(description);
                }

                // readd mybuddy to group if enabled
                if (isMyBuddyEnabled) {
                    group.getMembers().add(imbot);
                }

                if (jidsToAdd != null) {
                    for (JID jid : jidsToAdd) {
                        logger.debug("adding JID " + jid + " in group " + groupName);
                        Collection<Group> groups = GroupManager.getInstance().getGroups(jid);
                        for (Group g : groups) {
                            UserUpdateJob.removeUserFromGroup(g.getName(), jid);
                        }
                        for (Group g : groups) {
                            UserUpdateJob.addUserToGroup(g.getName(), jid);
                        }
                    }
                }
            } catch (GroupNotFoundException e) {
                logger.error("Group not found trying to remove mybuddy from " + groupName);
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
