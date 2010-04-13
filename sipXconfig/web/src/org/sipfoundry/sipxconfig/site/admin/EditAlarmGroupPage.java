/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.site.admin;

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.admin.alarm.AlarmGroup;
import org.sipfoundry.sipxconfig.admin.alarm.AlarmServerManager;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.permission.PermissionManager;
import org.sipfoundry.sipxconfig.site.user.SelectUsers;
import org.sipfoundry.sipxconfig.site.user.SelectUsersCallback;
import org.sipfoundry.sipxconfig.site.user.UserTable;
import org.sipfoundry.sipxconfig.site.user_portal.UserBasePage;

public abstract class EditAlarmGroupPage extends UserBasePage {
    public static final String PAGE = "admin/EditAlarmGroupPage";

    @InjectObject("spring:alarmServerManager")
    public abstract AlarmServerManager getAlarmServerManager();

    @InjectObject("spring:permissionManager")
    public abstract PermissionManager getPermissionManager();

    @Persist
    public abstract Integer getGroupId();

    public abstract void setGroupId(Integer groupId);

    @Persist
    public abstract AlarmGroup getGroup();

    public abstract void setGroup(AlarmGroup group);

    public abstract Collection getAddedUsers();

    public abstract void setAddedUsers(Collection addedUsers);

    public abstract Collection getAddedAddresses();

    public abstract void setAddedAddresses(Collection addedAddresses);

    public void addAlarmGroup(IPage returnPage) {
        setGroupId(null);
        setGroup(null);
        setReturnPage(returnPage);
    }

    public void editAlarmGroup(Integer groupId, IPage returnPage) {
        setGroupId(groupId);
        setGroup(null);
        setReturnPage(returnPage);
    }

    @Override
    public void pageBeginRender(PageEvent event_) {
        if (!TapestryUtils.isValid(this)) {
            return;
        }

        AlarmGroup group = getGroup();
        if (group == null) {
            Integer alarmGroupId = getGroupId();
            if (alarmGroupId == null) {
                group = new AlarmGroup();
            } else {
                group = getAlarmServerManager().getAlarmGroupById(alarmGroupId);
            }
            setGroup(group);
        } else {
            setGroupId(group.getId());
        }

        // Permission manager should be automatically restored when the User is
        // retrieved from Hibernate, but that's not happening for some reason.
        for (User user : group.getUsers()) {
            user.setPermissionManager(getPermissionManager());
        }

        if (getAddedUsers() != null && getAddedUsers().size() > 0) {
            group.getUsers().addAll(getUsersBySelectedIds(getAddedUsers()));
        }
    }

    public IPage addUsers(IRequestCycle cycle) {
        SelectUsers editUsers = (SelectUsers) cycle.getPage(SelectUsers.PAGE);
        SelectUsersCallback callback = new SelectUsersCallback(this.getPage());
        callback.setIdsPropertyName("addedUsers");
        editUsers.setCallback(callback);
        editUsers.setUseEmailAddress(true);
        editUsers.setTitle(getMessages().getMessage("title.selectRings"));
        editUsers.setPrompt(getMessages().getMessage("prompt.selectRings"));
        return editUsers;
    }

    public void delete() {
        UserTable usersTable = (UserTable) getComponent("extensionsTable");
        Collection selectedIdsToDelete = usersTable.getSelections().getAllSelected();
        getGroup().getUsers().removeAll(getUsersBySelectedIds(selectedIdsToDelete));
    }

    private List<User> getUsersBySelectedIds(Collection selectedIds) {
        List<User> users = new ArrayList<User>();
        for (Object obj : selectedIds) {
            User currentUser = getCoreContext().loadUser((Integer) obj);
            if (currentUser != null) {
                users.add(currentUser);
            }
        }
        return users;
    }

    public void commitGroup() {
        if (!TapestryUtils.isValid(this)) {
            return;
        }
        getAlarmServerManager().saveAlarmGroup(getGroup());
    }
}
