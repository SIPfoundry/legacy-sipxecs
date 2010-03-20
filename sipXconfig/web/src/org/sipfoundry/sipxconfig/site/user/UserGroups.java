/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.user;

import java.util.List;
import java.util.Map;

import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.components.SipxBasePage;
import org.sipfoundry.sipxconfig.setting.SettingDao;
import org.sipfoundry.sipxconfig.site.setting.GroupSettings;

public abstract class UserGroups extends SipxBasePage implements PageBeginRenderListener {

    public static final String PAGE = "user/UserGroups";

    public abstract void setGroups(List groups);

    public abstract List getGroups();

    public abstract CoreContext getCoreContext();

    public abstract SettingDao getSettingContext();

    public IPage addGroup(IRequestCycle cycle) {
        GroupSettings page = (GroupSettings) cycle.getPage(UserGroupSettings.PAGE);
        User user = getCoreContext().newUser();
        page.editGroup(null, user, PAGE);
        return page;
    }

    public Map getMemberCounts() {
        Map memberCount = getSettingContext().getGroupMemberCountIndexedByGroupId(User.class);
        return memberCount;
    }

    public IPage editUserGroup(IRequestCycle cycle, Integer groupId) {
        GroupSettings page = (GroupSettings) cycle.getPage(UserGroupSettings.PAGE);
        User user = getCoreContext().newUser();
        page.editGroup(groupId, user, PAGE);
        return page;
    }

    public IPage showGroupMembers(IRequestCycle cycle, Integer groupId) {
        ManageUsers page = (ManageUsers) cycle.getPage(ManageUsers.PAGE);
        page.setGroupId(groupId);
        return page;
    }

    public void pageBeginRender(PageEvent event_) {
        CoreContext context = getCoreContext();
        setGroups(context.getGroups());
    }
}
