/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.setting;

import java.util.Collection;
import java.util.Iterator;

import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.SettingDao;

public abstract class EditGroup extends PageWithCallback implements PageBeginRenderListener {

    public static final String PAGE = "setting/EditGroup";

    public abstract String getResource();

    public abstract void setResource(String resourceId);

    public abstract Group getGroup();

    public abstract void setGroup(Group group);

    public abstract Integer getGroupId();

    public abstract void setGroupId(Integer groupId);

    public abstract SettingDao getSettingContext();

    public abstract boolean getShowBranch();

    public abstract void setShowBranch(boolean showBranch);


    public static void saveGroups(SettingDao dao, Collection groups) {
        for (Iterator i = groups.iterator(); i.hasNext();) {
            dao.saveGroup((Group) i.next());
        }
    }

    public void pageBeginRender(PageEvent event_) {
        Group group = getGroup();
        if (group != null) {
            return;
        }
        Integer groupId = getGroupId();
        if (groupId != null) {
            group = getSettingContext().loadGroup(groupId);
        } else {
            group = new Group();
            group.setResource(getResource());
        }
        setGroup(group);
    }

    public void newGroup(String resourceId, String returnPage) {
        setResource(resourceId);
        setGroupId(null);
        setReturnPage(returnPage);
    }

    public void editGroup(Integer groupId, String returnPage) {
        setResource(null);
        setGroupId(groupId);
        setReturnPage(returnPage);
    }

    /*
     * If the input is valid, then save changes to the group.
     */
    public void apply() {
        if (!TapestryUtils.isValid(this)) {
            return;
        }
        Group group = getGroup();
        getSettingContext().saveGroup(group);
        setGroupId(group.getId());
    }
}
