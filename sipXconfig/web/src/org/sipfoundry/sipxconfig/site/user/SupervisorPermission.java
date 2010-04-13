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

import java.util.Collection;
import java.util.List;
import java.util.Set;

import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.setting.BeanWithGroups;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.SettingDao;

public abstract class SupervisorPermission extends PageWithCallback implements
        PageBeginRenderListener {
    public static final String PAGE = "user/SupervisorPermission";

    public abstract Integer getUserId();

    public abstract void setUserId(Integer userId);

    public abstract User getUser();

    public abstract void setUser(User user);

    public abstract CoreContext getCoreContext();

    public abstract SettingDao getSettingDao();

    public abstract String getSupervisorForGroupsString();

    public abstract void setSupervisorForGroupsString(String groups);

    public abstract void setGroupCandidates(Collection groupsList);

    public void buildGroupCandidates(String groupsString) {
        List allGroups = getCoreContext().getGroups();
        Collection candidates = TapestryUtils.getAutoCompleteCandidates(allGroups, groupsString);
        setGroupCandidates(candidates);
    }

    public void pageBeginRender(PageEvent event_) {
        User user = getUser();
        if (user == null) {
            user = getCoreContext().loadUser(getUserId());
            setUser(user);

            Set groups = user.getSupervisorForGroups();
            String groupsString = BeanWithGroups.getGroupsAsString(groups);
            setSupervisorForGroupsString(groupsString);
        }
    }

    public void commit() {
        User user = getUser();

        String groupsString = getSupervisorForGroupsString();
        user.clearSupervisorForGroups();
        if (groupsString != null) {
            List<Group> groupList = getSettingDao().getGroupsByString(User.GROUP_RESOURCE_ID,
                    groupsString, true);
            for (Group g : groupList) {
                user.addSupervisorForGroup(g);
            }
        }

        getCoreContext().saveUser(user);
    }
}
