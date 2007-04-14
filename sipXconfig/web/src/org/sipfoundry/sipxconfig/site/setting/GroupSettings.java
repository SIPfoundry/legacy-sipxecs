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

import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.setting.BeanWithGroups;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingDao;

public abstract class GroupSettings extends BasePage implements PageBeginRenderListener {

    public static final String PAGE = "GroupSettings";

    public abstract Integer getGroupId();

    public abstract void setGroupId(Integer id);

    public abstract Group getGroup();

    public abstract void setGroup(Group group);

    public abstract String getParentSettingName();

    public abstract void setParentSettingName(String name);

    public abstract Setting getParentSetting();

    public abstract void setParentSetting(Setting parent);

    public abstract SettingDao getSettingDao();

    public abstract void setSettings(Setting setting);

    public abstract Setting getSettings();

    public abstract BeanWithGroups getBean();

    public abstract void setBean(BeanWithGroups bean);

    public abstract void setReturnPage(String returnPage);

    public abstract String getReturnPage();

    public IPage editGroupName(IRequestCycle cycle) {
        EditGroup page = (EditGroup) cycle.getPage(EditGroup.PAGE);
        page.editGroup(getGroupId(), PAGE);
        return page;
    }

    public void editGroup(Integer groupId, BeanWithGroups bean, String returnPage) {
        setBean(bean);
        setGroupId(groupId);
        setReturnPage(returnPage);
    }

    @SuppressWarnings("unused")
    public void editGroupSettings(Integer beanId, String settingName) {
        setParentSettingName(settingName);
    }

    public void pageBeginRender(PageEvent event_) {
        Group group = getGroup();
        if (group != null) {
            return;
        }

        group = getSettingDao().getGroup(getGroupId());
        setGroup(group);
        Setting settings = group.inherhitSettingsForEditing(getBean());
        setSettings(settings);
        String currentSettingName = getParentSettingName();
        if (currentSettingName == null) {
            Setting first = settings.getValues().iterator().next();
            currentSettingName = first.getName();
        }
        Setting parent = settings.getSetting(currentSettingName);
        setParentSetting(parent);
    }

    public String ok() {
        apply();
        return getReturnPage();
    }

    public void apply() {
        SettingDao dao = getSettingDao();
        dao.saveGroup(getGroup());
    }

    public String cancel() {
        return getReturnPage();
    }
}
