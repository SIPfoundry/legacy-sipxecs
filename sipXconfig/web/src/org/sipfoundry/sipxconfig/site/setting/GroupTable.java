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

import java.util.Iterator;
import java.util.List;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IAsset;
import org.apache.tapestry.annotations.Asset;
import org.sipfoundry.sipxconfig.common.DataCollectionUtil;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.SettingDao;

public abstract class GroupTable extends BaseComponent {

    @Asset("/images/group.png")
    public abstract IAsset getGroupIcon();
    
    public abstract SettingDao getSettingContext();

    public abstract List getGroups();

    public abstract SelectMap getSelections();

    public void deleteGroup() {
        SettingDao context = getSettingContext();
        context.deleteGroups(getSelections().getAllSelected());
    }

    public void moveUp() {
        moveGroups(-1);
    }

    public void moveDown() {
        moveGroups(1);
    }

    void moveGroups(int step) {
        SettingDao context = getSettingContext();
        Object[] groupIds = getSelections().getAllSelected().toArray();
        List groups = getGroups();
        DataCollectionUtil.moveByPrimaryKey(groups, groupIds, step);
        Iterator i = groups.iterator();
        while (i.hasNext()) {
            Group group = (Group) i.next();
            context.saveGroup(group);
        }
    }
}
