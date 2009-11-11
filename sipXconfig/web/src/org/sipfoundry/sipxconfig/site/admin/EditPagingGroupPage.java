/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
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
import org.apache.tapestry.valid.ValidatorException;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.paging.PagingContext;
import org.sipfoundry.sipxconfig.paging.PagingGroup;
import org.sipfoundry.sipxconfig.service.SipxPageService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;
import org.sipfoundry.sipxconfig.site.user.SelectUsers;
import org.sipfoundry.sipxconfig.site.user.SelectUsersCallback;
import org.sipfoundry.sipxconfig.site.user.UserTable;
import org.sipfoundry.sipxconfig.site.user_portal.UserBasePage;

public abstract class EditPagingGroupPage extends UserBasePage {
    public static final String PAGE = "admin/EditPagingGroupPage";

    @Override
    @InjectObject(value = "spring:coreContext")
    public abstract CoreContext getCoreContext();

    @InjectObject(value = "spring:pagingContext")
    public abstract PagingContext getPagingContext();

    @InjectObject(value = "spring:sipxServiceManager")
    public abstract SipxServiceManager getSipxServiceManager();

    @Persist(value = "client")
    public abstract Integer getGroupId();

    public abstract void setGroupId(Integer groupId);

    @Persist
    public abstract PagingGroup getGroup();

    public abstract void setGroup(PagingGroup group);

    public abstract Collection getAddedUsers();

    public abstract void setAddedUsers(Collection addedUsers);

    public void addPagingGroup(String returnPage) {
        setGroup(null);
        setReturnPage(returnPage);
    }

    public void editPagingGroup(Integer groupId, String returnPage) {
        setGroupId(groupId);
        setGroup(getPagingContext().getPagingGroupById(groupId));
        setReturnPage(returnPage);
    }

    @Override
    public void pageBeginRender(PageEvent event_) {

        if (!TapestryUtils.isValid(this)) {
            return;
        }

        if (getGroup() == null && getGroupId() == null) {
            setGroup(new PagingGroup());
        }

        PagingGroup group = getGroup();

        String sound = group.getSound();
        if (sound == null) {
            sound = "notice.wav";
            group.setSound(sound);
        }

        if (getAddedUsers() != null && getAddedUsers().size() > 0) {
            group.getUsers().addAll(getUsersBySelectedIds(getAddedUsers()));
        }
        setGroup(group);
    }

    public SipxPageService getSipxPageService() {
        return ((SipxPageService) getSipxServiceManager().getServiceByBeanId(SipxPageService.BEAN_ID));
    }

    public IPage add(IRequestCycle cycle) {
        SelectUsers editUsers = (SelectUsers) cycle.getPage(SelectUsers.PAGE);
        SelectUsersCallback callback = new SelectUsersCallback(this.getPage());
        callback.setIdsPropertyName("addedUsers");
        editUsers.setCallback(callback);
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
            users.add(getCoreContext().loadUser((Integer) obj));
        }
        return users;
    }

    public void commit() {
        if (!TapestryUtils.isValid(this)) {
            return;
        }

        try {
            getPagingContext().savePagingGroup(getGroup());
        } catch (UserException ex) {
            String msg = getMessages().format("error.duplicateGroupNumbers",
                    getGroup().getPageGroupNumber());
            getValidator().record(new ValidatorException(msg));
        }
    }
}
