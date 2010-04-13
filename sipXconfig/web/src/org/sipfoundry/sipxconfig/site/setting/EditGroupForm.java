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

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IBinding;
import org.apache.tapestry.annotations.Parameter;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.SettingDao;

public abstract class EditGroupForm extends BaseComponent {

    public abstract Group getGroup();

    public abstract void setGroup(Group group);

    public abstract SettingDao getSettingContext();

    @Parameter (required = false, defaultValue = "true")
    public abstract boolean getShowBranch();

    public abstract void setShowBranch(boolean showBranch);


    // set the "group_id" parameter to TRUE
    private void markGroupId(int id) {
        IBinding groupId = getBinding("groupId");
        if (groupId != null) {
            groupId.setObject(new Integer(id));
        }
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
        markGroupId(group.getId());
    }
}
