/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.site.setting;

import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.setting.BeanWithGroups;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingDao;

public abstract class GroupSettings extends PageWithCallback implements PageBeginRenderListener {
    @Persist()
    public abstract Integer getGroupId();

    public abstract void setGroupId(Integer id);

    public abstract Group getGroup();

    public abstract void setGroup(Group group);

    @Persist()
    public abstract String getParentSettingName();

    public abstract void setParentSettingName(String name);

    public abstract Setting getParentSetting();

    public abstract void setParentSetting(Setting parent);

    @InjectObject(value = "spring:settingDao")
    public abstract SettingDao getSettingDao();

    public abstract void setSettings(Setting setting);

    public abstract Setting getSettings();

    @Persist()
    public abstract BeanWithGroups getBean();

    public abstract void setBean(BeanWithGroups bean);

    @Bean()
    public abstract SipxValidationDelegate getValidator();

    public IPage editGroupName(IRequestCycle cycle) {
        EditGroup page = (EditGroup) cycle.getPage(EditGroup.PAGE);
        page.editGroup(getGroupId(), getPageName());
        return page;
    }

    public void editGroup(Integer groupId, BeanWithGroups bean, String returnPage) {
        setBean(bean);
        setGroupId(groupId);
        setReturnPage(returnPage);
    }

    public void editGroupSettings(@SuppressWarnings("unused") Integer beanId, String settingName) {
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

    public void ok(IRequestCycle cycle) {
        apply();
        getCallback().performCallback(cycle);
    }

    public void apply() {
        SettingDao dao = getSettingDao();
        dao.saveGroup(getGroup());
    }

    public void cancel(IRequestCycle cycle) {
        getCallback().performCallback(cycle);
    }
}
