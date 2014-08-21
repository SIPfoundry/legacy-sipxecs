package org.sipfoundry.openfire.plugin.job.user;

import static org.sipfoundry.commons.mongo.MongoConstants.UID;

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.lang.StringUtils;
import org.apache.log4j.Logger;
import org.jivesoftware.openfire.group.Group;
import org.jivesoftware.openfire.group.GroupManager;
import org.jivesoftware.openfire.group.GroupNotFoundException;
import org.jivesoftware.openfire.user.User;
import org.jivesoftware.openfire.user.UserManager;
import org.jivesoftware.openfire.user.UserNotFoundException;
import org.sipfoundry.openfire.sync.job.Job;
import org.xmpp.packet.JID;

public class UserUpdateJob implements Job {
    private static Logger logger = Logger.getLogger(UserUpdateJob.class);

    private static final long serialVersionUID = 1L;

    private final String userImName;
    private final String oldUserImName;
    private final boolean isImUser;
    private final String displayName;
    private final String email;
    private final String uid;
    private final List<String> groups;

    public UserUpdateJob(String userImName, String oldUserImName, boolean isImUser, String displayName,
            String email, String uid, List<String> groups) {
        this.userImName = userImName;
        this.oldUserImName = oldUserImName;
        this.isImUser = isImUser;
        this.displayName = displayName;
        this.email = email;
        this.uid = uid;
        this.groups = groups;
    }

    @Override
    public void process() {
        logger.debug("start processing " + toString());

        // check if still an IM User, if not delete it
        // not imuser and in cache: delete it
        if (!isImUser && !StringUtils.isBlank(userImName)) {
            UserShared.removeUser(userImName);
            return;
        }

        try {
            User user = UserManager.getInstance().getUser(oldUserImName);

            // imuser and in cache: update it
            if (isImUser && !StringUtils.isBlank(userImName)) {
                if (!StringUtils.equalsIgnoreCase(oldUserImName, userImName)) {
                    logger.debug(String.format("im id changed to %s from %s", userImName, oldUserImName));
                    // old name will be null for users that just had im enabled - nothing to
                    // remove
                    List<String> actualGroups = new ArrayList<String>();
                    if (oldUserImName != null) {
                        Collection<Group> groups = GroupManager.getInstance().getGroups(
                                UserManager.getInstance().getUser(userImName));
                        for (Group group : groups) {
                            actualGroups.add(group.getName());
                        }
                        UserShared.removeUser(oldUserImName);
                    }
                    updateGroups(actualGroups, userImName);
                    return;
                }
            }

            // group update should go first
            if (groups != null) {
                updateGroups(groups, userImName);
            }

            // update display name & email if changed

            boolean dnChanged = displayName != null && !StringUtils.equals(user.getName(), displayName);
            boolean emailChanged = email != null && !StringUtils.equals(user.getEmail(), email);
            boolean uidChanged = uid != null && !StringUtils.equals(user.getProperties().get(UID), uid);
            boolean userChanged = dnChanged || emailChanged || uidChanged;

            if (dnChanged) {
                logger.debug(String.format("im name changed to %s from %s", displayName, user.getName()));
                user.setName(displayName);
                userChanged = true;
            }
            if (emailChanged) {
                logger.debug(String.format("email changed to %s from %s", email, user.getEmail()));
                user.setEmail(email);
                userChanged = true;
            }

            if (uidChanged) {
                logger.debug(String.format("uid changed to %s", uid));
                user.getProperties().put(UID, uid);
                userChanged = true;
            }
            if (userChanged) {
                logger.debug("User changed; updating roster: " + userImName);
                // We need to remove user from all the groups and add it back in order for the
                // rosters to be updated properly.
                // More, we need to remove it first and then readd it, otherwise, if it is in more
                // than 1 im group, the rosters are not updated.
                JID jid = new JID(UserShared.appendDomain(userImName));
                Collection<Group> groups = GroupManager.getInstance().getGroups(jid);
                for (Group g : groups) {
                    removeUserFromGroup(g.getName(), jid);
                }
                for (Group g : groups) {
                    addUserToGroup(g.getName(), jid);
                }
            }
        } catch (UserNotFoundException e) {
            logger.error("User not found trying to update " + oldUserImName);
        }
        logger.debug("end processing " + toString());
    }

    private static void updateGroups(List<String> actualGroups, String imName) {
        // rebuild groups in openfire
        JID jid = new JID(UserShared.appendDomain(imName));

        // get groups already assigned for current JID
        List<String> assignedGroups = new ArrayList<String>();
        for (Group group : GroupManager.getInstance().getGroups()) {
            if (group.getMembers().contains(jid) || group.getAdmins().contains(jid)) {
                assignedGroups.add(group.getName());
            }
        }

        // groups that were deleted
        @SuppressWarnings("unchecked")
        Collection<String> groupsDeleted = CollectionUtils.subtract(assignedGroups, actualGroups);
        for (String groupName : groupsDeleted) {
            removeUserFromGroup(groupName, jid);
        }

        // groups that were added
        @SuppressWarnings("unchecked")
        Collection<String> groupsAdded = CollectionUtils.subtract(actualGroups, assignedGroups);
        for (String groupName : groupsAdded) {
            addUserToGroup(groupName, jid);
        }
    }

    public static void removeUserFromGroup(String groupName, JID jid) {
        logger.debug(String.format("remove user %s from group %s", jid.toString(), groupName));
        try {
            if (StringUtils.isNotBlank(groupName)) {
                Group group = GroupManager.getInstance().getGroup(groupName);
                group.getMembers().remove(jid);
                // it's not supposed to be an admin, but just in case
                group.getAdmins().remove(jid);
            }
        } catch (GroupNotFoundException ex) {
            logger.error(String.format("Remove user %s from group %s: Group not found", jid.toString(), groupName));
        }
    }

    public static void addUserToGroup(String groupName, JID jid) {
        logger.debug(String.format("Add user %s to group %s", jid.toString(), groupName));
        try {
            if (StringUtils.isNotBlank(groupName)) {
                Group group = GroupManager.getInstance().getGroup(groupName);
                // automatically add the user in group as admin. Openfire holds two collections
                // members collection and admins collection. A user is contained either in members
                // or in admins collection, not in both
                group.getMembers().add(jid);
            }
        } catch (GroupNotFoundException ex) {
            logger.error(String.format("Add user %s to group %s: Group not found", jid.toString(), groupName));
        }
    }

    @Override
    public String toString() {
        return "UserUpdateJob [userImName=" + userImName + ", oldUserImName=" + oldUserImName + ", isImUser="
                + isImUser + ", displayName=" + displayName + ", email=" + email + ", uid=" + uid + ", groups="
                + groups + "]";
    }

    @Override
    public int hashCode() {
        final int prime = 31;
        int result = 1;
        result = prime * result + ((displayName == null) ? 0 : displayName.hashCode());
        result = prime * result + ((email == null) ? 0 : email.hashCode());
        result = prime * result + ((groups == null) ? 0 : groups.hashCode());
        result = prime * result + (isImUser ? 1231 : 1237);
        result = prime * result + ((oldUserImName == null) ? 0 : oldUserImName.hashCode());
        result = prime * result + ((uid == null) ? 0 : uid.hashCode());
        result = prime * result + ((userImName == null) ? 0 : userImName.hashCode());
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
        UserUpdateJob other = (UserUpdateJob) obj;
        if (displayName == null) {
            if (other.displayName != null) {
                return false;
            }
        } else if (!displayName.equals(other.displayName)) {
            return false;
        }
        if (email == null) {
            if (other.email != null) {
                return false;
            }
        } else if (!email.equals(other.email)) {
            return false;
        }
        if (groups == null) {
            if (other.groups != null) {
                return false;
            }
        } else if (!groups.equals(other.groups)) {
            return false;
        }
        if (isImUser != other.isImUser) {
            return false;
        }
        if (oldUserImName == null) {
            if (other.oldUserImName != null) {
                return false;
            }
        } else if (!oldUserImName.equals(other.oldUserImName)) {
            return false;
        }
        if (uid == null) {
            if (other.uid != null) {
                return false;
            }
        } else if (!uid.equals(other.uid)) {
            return false;
        }
        if (userImName == null) {
            if (other.userImName != null) {
                return false;
            }
        } else if (!userImName.equals(other.userImName)) {
            return false;
        }
        return true;
    }
}
