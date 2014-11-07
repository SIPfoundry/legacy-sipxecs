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

import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;

import org.apache.commons.lang.StringUtils;
import org.apache.tapestry.IComponent;
import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.apache.tapestry.web.WebContext;
import org.sipfoundry.sipxconfig.components.LocalizationUtils;
import org.sipfoundry.sipxconfig.device.DeviceVersion;
import org.sipfoundry.sipxconfig.hoteling.HotelingLocator;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneModel;
import org.sipfoundry.sipxconfig.setting.BeanWithSettings;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingDao;
import org.sipfoundry.sipxconfig.setting.SettingFilter;
import org.sipfoundry.sipxconfig.setting.SettingUtil;
import org.sipfoundry.sipxconfig.setting.type.EnumSetting;

public abstract class EditPhoneDefaults extends PhoneBasePage implements PageBeginRenderListener {
    private static final String GROUP_VERSION_FIRMWARE_VERSION = "group.version/firmware.version";

    public static final String PAGE = "phone/EditPhoneDefaults";

    public static final int FW_TAB = -1;

    public static final int PHONE_SETTINGS = 0;

    private static final int LINE_SETTITNGS = 1;

    public static final int GROUP_VERSION = 2;

    @InjectObject(value = "spring:hotelingLocator")
    public abstract HotelingLocator getHotellingLocator();

    @InjectObject(value = "spring:settingDao")
    public abstract SettingDao getSettingDao();

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

    @InjectObject(value = "service:tapestry.globals.WebContext")
    public abstract WebContext getWebContext();

    public abstract void setGroupVersions(IPropertySelectionModel model);

    @Persist(value = "client")
    public abstract void setGroupVersion(String gv);

    public abstract String getGroupVersion();

    public abstract IPropertySelectionModel getGroupVersions();

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

    @Override
    public void pageBeginRender(PageEvent event) {
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

        // Init. the group versions dropdown menu.
        if (hasGroupVersions()) {
            if (getGroupVersions() == null) {
                Map<String, String> versions = ((EnumSetting) getGroup().inherhitSettingsForEditing(phone)
                        .getSetting(GROUP_VERSION_FIRMWARE_VERSION).getType()).getEnums();
                List<DeviceVersion> deviceVersions = new ArrayList<DeviceVersion>();
                for (String version : versions.keySet()) {
                    if (StringUtils.isNotBlank(version)) {
                        DeviceVersion dv = DeviceVersion.getDeviceVersion(phone.getBeanId() + version);
                        deviceVersions.add(dv);
                    }
                }
                IPropertySelectionModel versionsSelectionModel = new GroupVersionsSelectionModel(deviceVersions);
                setGroupVersions(versionsSelectionModel);
            }
            String groupVersion = getGroupVersion();
            if (groupVersion == null) {
                setGroupVersion(getGroup().inherhitSettingsForEditing(getPhone())
                        .getSetting(GROUP_VERSION_FIRMWARE_VERSION).getValue());
            } else {
                getGroup().inherhitSettingsForEditing(getPhone()).getSetting(GROUP_VERSION_FIRMWARE_VERSION)
                        .setValue(groupVersion);
            }
            if (!event.getRequestCycle().isRewinding()) {
                if (getResourceId() != GROUP_VERSION) {
                    setGroupVersion(null);
                }
            }
        }
        editSettings();
    }

    public boolean hasGroupVersions() {
        return getGroup().inherhitSettingsForEditing(getPhone()).getSetting(GROUP_VERSION_FIRMWARE_VERSION) != null;
    }

    public void applyGroupVersion() {
        getPhoneContext().applyGroupFirmwareVersion(getGroup(),
                DeviceVersion.getDeviceVersion(getPhone().getBeanId() + getGroupVersion()));
    }

    /**
     * Based on current (persistent) page state, setup the settings data for the setting edit form
     */
    public void editSettings() {
        BeanWithSettings bean;
        if (getResourceId() == FW_TAB || getResourceId() == GROUP_VERSION) {
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
            // resetting navigation each time you change version is an
            // inconvience.
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

    public class GroupVersionsSelectionModel implements IPropertySelectionModel {
        List<DeviceVersion> m_versions = new ArrayList<DeviceVersion>();

        public GroupVersionsSelectionModel(List<DeviceVersion> versions) {
            m_versions = versions;
        }

        @Override
        public String getLabel(int i) {
            return m_versions.get(i).getVersionId();
        }

        @Override
        public Object getOption(int i) {
            return m_versions.get(i).getName();
        }

        @Override
        public int getOptionCount() {
            return m_versions.size();
        }

        @Override
        public String getValue(int i) {
            return m_versions.get(i).getVersionId();
        }

        @Override
        public boolean isDisabled(int arg0) {
            return false;
        }

        @Override
        public Object translateValue(String value) {
            return value;
        }

    }

    @Override
    public String getBorderTitle() {
        return getPhoneModel().getLabel();
    }

    public void editFirmwareVersion() {
        setEditFormSetting(null);
        setResourceId(FW_TAB);
    }

    public void editGroupVersion() {
        setEditFormSetting(null);
        setResourceId(GROUP_VERSION);
    }

    public IComponent getDisplayFwverTab() {
        if (getResourceId() == FW_TAB) {
            return (IComponent) getComponents().get("fwverTab");
        }
        return null;

    }

    public IComponent getDisplayGroupVersionTab() {
        if (getResourceId() == GROUP_VERSION) {
            return (IComponent) getComponents().get("groupVersionTab");
        }
        return null;

    }

    public String getGroupsToHide() {
        List<String> names = new LinkedList<String>();
        if (!isHotellingEnabled()) {
            names.add("prov");
        }
        return StringUtils.join(names, ",");
    }

    public boolean isHotellingEnabled() {
        return getHotellingLocator().isHotellingEnabled();
    }
}
