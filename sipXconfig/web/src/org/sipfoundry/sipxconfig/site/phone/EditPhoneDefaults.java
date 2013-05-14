/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.phone;

import java.util.Collection;
import java.util.Iterator;

import org.apache.tapestry.IComponent;
import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.components.LocalizationUtils;
import org.sipfoundry.sipxconfig.components.SipxBasePage;
import org.sipfoundry.sipxconfig.device.DeviceVersion;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.phone.PhoneModel;
import org.sipfoundry.sipxconfig.setting.BeanWithSettings;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingDao;
import org.sipfoundry.sipxconfig.setting.SettingFilter;
import org.sipfoundry.sipxconfig.setting.SettingUtil;

public abstract class EditPhoneDefaults extends SipxBasePage implements PageBeginRenderListener {

    public static final String PAGE = "phone/EditPhoneDefaults";

    private static final int FW_TAB = -1;

    private static final int PHONE_SETTINGS = 0;

    private static final int LINE_SETTITNGS = 1;

    @InjectObject(value = "spring:settingDao")
    public abstract SettingDao getSettingDao();

    @InjectObject(value = "spring:phoneContext")
    public abstract PhoneContext getPhoneContext();

    public abstract void setPhone(Phone phone);

    public abstract Phone getPhone();

    @Persist
    public abstract PhoneModel getPhoneModel();

    public abstract void setPhoneModel(PhoneModel model);

    public abstract Group getGroup();

    public abstract void setGroup(Group group);

    @Persist
    public abstract Integer getGroupId();

    public abstract void setGroupId(Integer id);

    public abstract Setting getCurrentNavigationSetting();

    public abstract Setting getEditFormSetting();

    public abstract void setEditFormSetting(Setting setting);

    public abstract void setEditFormSettings(Collection settings);

    @Persist
    public abstract String getEditFormSettingName();

    public abstract void setEditFormSettingName(String name);

    @Persist
    public abstract void setResourceId(int resource);

    public abstract int getResourceId();

    @Persist(value = "client")
    public abstract void setDeviceVersion(DeviceVersion version);

    public abstract DeviceVersion getDeviceVersion();

    /**
     * Entry point for other pages to edit a phone model's default settings
     */
    public void editPhoneSettings(PhoneModel phoneModel, Integer groupId) {
        setPhoneModel(phoneModel);
        setEditFormSettingName(null);
        setGroupId(groupId);
    }

    public Collection getPhoneNavigationSettings() {
        return getPhone().getSettings().getValues();
    }

    public Line getLine() {
        return getPhone().getLine(0);
    }

    public Collection getLineNavigationSettings() {
        return getLine().getSettings().getValues();
    }

    public IPage editPhoneSettingsClicked(@SuppressWarnings("unused") Integer beanId, String settingName) {
        setResourceId(PHONE_SETTINGS);
        setEditFormSettingName(settingName);
        return getPage();
    }

    public IPage editLineSettingsClicked(@SuppressWarnings("unused") Integer beanId, String settingName) {
        setResourceId(LINE_SETTITNGS);
        setEditFormSettingName(settingName);
        return getPage();
    }

    public IPage ok(IRequestCycle cycle) {
        apply();
        return getReturnPage(cycle);
    }

    public void apply() {
        getSettingDao().saveGroup(getGroup());
    }

    public IPage cancel(IRequestCycle cycle) {
        return getReturnPage(cycle);
    }

    private IPage getReturnPage(IRequestCycle cycle) {
        PhoneModels page = (PhoneModels) cycle.getPage(PhoneModels.PAGE);
        page.setGroupId(getGroupId());
        return page;
    }

    public void pageBeginRender(PageEvent event_) {
        if (getPhoneModel() == null) {
            throw new IllegalArgumentException("phone factory id required");
        }

        Group group = getGroup();
        group = getSettingDao().loadGroup(getGroupId());
        setGroup(group);

        Phone phone = getPhone();
        phone = getPhoneContext().newPhone(getPhoneModel());
        Line line = phone.createLine();
        phone.addLine(line);
        setPhone(phone);

        DeviceVersion deviceVersion = getDeviceVersion();
        if (deviceVersion == null) {
            setDeviceVersion(phone.getDeviceVersion());
        } else {
            phone.setDeviceVersion(deviceVersion);
        }

        String editSettingsName = getEditFormSettingName();
        if (editSettingsName == null) {
            setResourceId(-1);
            Iterator nav = getPhoneNavigationSettings().iterator();
            setEditFormSettingName(((Setting) nav.next()).getName());
        }

        editSettings();
    }

    /**
     * Based on current (persistent) page state, setup the settings data for the setting edit form
     */
    private void editSettings() {
        BeanWithSettings bean;
        if (getResourceId() == FW_TAB) {
            return;
        } else if (getResourceId() == PHONE_SETTINGS) {
            bean = getPhone();
        } else {
            bean = getLine();
        }

        Setting settings = getGroup().inherhitSettingsForEditing(bean);
        Setting subset = settings.getSetting(getEditFormSettingName());
        if (subset == null) {
            // Only time this is true is if navigation on an item that doesn't
            // exist anymore because a a new firmware version was selected. IMO
            // resetting navigation each time you change version is an inconvience.
            subset = settings.getValues().iterator().next();
            setEditFormSettingName(subset.getName());
        }
        setEditFormSetting(subset);

        setEditFormSettings(SettingUtil.filter(SettingFilter.ALL, subset));
    }

    public String getEditFormSettingLabel() {
        return LocalizationUtils.getSettingLabel(this, getEditFormSetting());
    }

    public String getEditFormSettingDescription() {
        if (getEditFormSetting() == null) {
            return null;
        }
        return LocalizationUtils.getSettingDescription(this, getEditFormSetting());
    }

    public String getCurrentNavigationSettingLabel() {
        if (getEditFormSetting() == null) {
            return null;
        }
        return LocalizationUtils.getSettingLabel(this, getCurrentNavigationSetting());
    }

    public Setting getLineActiveSetting() {
        return LINE_SETTITNGS == getResourceId() ? getEditFormSetting() : null;
    }

    public Setting getPhoneActiveSetting() {
        return PHONE_SETTINGS == getResourceId() ? getEditFormSetting() : null;
    }

    @Override
    public String getBorderTitle() {
        return getPhoneModel().getLabel();
    }

    public void editFirmwareVersion() {
        setEditFormSetting(null);
        setResourceId(FW_TAB);
    }

    public IComponent getDisplayFwverTab() {
        if (getResourceId() == FW_TAB) {
            return (IComponent) getComponents().get("fwverTab");
        } else {
            return null;
        }
    }
}
