/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.site.user;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.LinkedList;
import java.util.List;

import org.apache.commons.lang.StringUtils;
import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.InitialValue;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.apache.tapestry.form.StringPropertySelectionModel;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.conference.ConferenceBridgeContext;
import org.sipfoundry.sipxconfig.device.HotellingManager;
import org.sipfoundry.sipxconfig.device.ProfileManager;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.forwarding.ForwardingContext;
import org.sipfoundry.sipxconfig.forwarding.UserGroupSchedule;
import org.sipfoundry.sipxconfig.imbot.ImBot;
import org.sipfoundry.sipxconfig.ivr.Ivr;
import org.sipfoundry.sipxconfig.permission.Permission;
import org.sipfoundry.sipxconfig.permission.PermissionName;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.site.setting.EditGroup;
import org.sipfoundry.sipxconfig.site.setting.EditSchedule;
import org.sipfoundry.sipxconfig.site.setting.GroupSettings;
import org.sipfoundry.sipxconfig.speeddial.SpeedDialGroup;
import org.sipfoundry.sipxconfig.speeddial.SpeedDialManager;
import org.sipfoundry.sipxconfig.time.NtpManager;
import org.sipfoundry.sipxconfig.vm.MailboxManager;

import com.davekoelle.AlphanumComparator;

public abstract class UserGroupSettings extends GroupSettings {
    public static final String PAGE = "user/UserGroupSettings";

    private static final String SEPARATOR = ",";

    private static final String SCHEDULES = "schedules";
    private static final String CONFERENCE = "conference";
    private static final String EXTCONTACT = "extcontact";
    private static final String SPEEDDIAL = "speeddial";
    private static final String CONFIGURE = "configure";
    private static final String VOICEMAIL = "voicemail";
    private static final String MOH = "moh";
    private static final String TIMEZONE_TAB = "timezone";
    private static final String TIMEZONE_SETTING = "timezone/timezone";
    private static final String HOTELLING_TAB = "hotelling";
    private static final String HOTELLING_SETTING = "hotelling/enable";

    @InjectObject(value = "spring:forwardingContext")
    public abstract ForwardingContext getForwardingContext();

    @InjectObject(value = "spring:conferenceBridgeContext")
    public abstract ConferenceBridgeContext getConferenceBridgeContext();

    @InjectObject("spring:featureManager")
    public abstract FeatureManager getFeatureManager();

    @InjectObject("spring:ntpManager")
    public abstract NtpManager getTimeManager();

    public abstract void setSchedules(List<UserGroupSchedule> schedules);

    public abstract List<UserGroupSchedule> getSchedules();

    public abstract boolean getChanged();

    @Persist
    public abstract String getTabName();

    public abstract void setTabName(String name);

    @InjectObject(value = "spring:mailboxManager")
    public abstract MailboxManager getMailboxManager();

    @InjectObject(value = "spring:speedDialManager")
    public abstract SpeedDialManager getSpeedDialManager();

    @InjectObject(value = "spring:phoneContext")
    public abstract PhoneContext getPhoneContext();

    @InjectObject(value = "spring:phoneProfileManager")
    public abstract ProfileManager getProfileManager();

    @InjectObject(value = "spring:coreContext")
    public abstract CoreContext getCoreContext();

    @Persist
    public abstract SpeedDialGroup getSpeedDialGroup();

    public abstract void setSpeedDialGroup(SpeedDialGroup speedDialGroup);

    public abstract String getTimezoneType();

    public abstract void setTimezoneType(String type);

    public abstract IPropertySelectionModel getTimezoneTypeModel();

    public abstract void setTimezoneTypeModel(IPropertySelectionModel model);

    public abstract Setting getHotellingSetting();

    public abstract void setHotellingSetting(Setting value);

    @Persist
    public abstract boolean getIsTabsSelected();

    public abstract void setIsTabsSelected(boolean enabled);

    @Persist
    @InitialValue("true")
    public abstract boolean getFirstRun();

    public abstract void setFirstRun(boolean enabled);

    public abstract void setValidationEnabled(boolean enabled);

    public abstract boolean isValidationEnabled();

    @Persist
    public abstract String getTab();

    public abstract void setTab(String tab);

    @InjectObject("spring:hotellingManager")
    public abstract HotellingManager getHotellingManager();

    public Collection<String> getAvailableTabNames() {
        Collection<String> tabNames = new ArrayList<String>();
        tabNames.add(CONFIGURE);
        if (isVoicemailEnabled()) {
            tabNames.add(VOICEMAIL);
        }
        tabNames.addAll(Arrays.asList(SCHEDULES, CONFERENCE, EXTCONTACT, SPEEDDIAL, TIMEZONE_TAB, HOTELLING_TAB));
        if (isVoicemailEnabled()) {
            tabNames.add(MOH);
        }
        return tabNames;
    }

    private boolean isVoicemailEnabled() {
        return (getFeatureManager().isFeatureEnabled(Ivr.FEATURE) ? true : false);
    }

    @Override
    public IPage editGroupName(IRequestCycle cycle) {
        EditGroup page = (EditGroup) cycle.getPage(EditGroup.PAGE);
        page.editGroup(getGroupId(), PAGE);
        return page;
    }

    @Override
    public void pageBeginRender(PageEvent event_) {
        if (null == getGroupId()) {
            Group group = new Group();
            group.setResource(User.GROUP_RESOURCE_ID);
            setGroup(group);
            setTab(CONFIGURE);
            setParentSetting(null);
            setParentSettingName(null);
            Setting settings = group.inherhitSettingsForEditing(getBean());
            setSettings(settings);
            setIsTabsSelected(true);
            setFirstRun(false);
            return;
        }

        Group group = getGroup();

        if (getChanged()) {
            setSchedules(null);
        }

        if (getSchedules() == null) {
            ForwardingContext forwardingContext = getForwardingContext();
            List<UserGroupSchedule> schedules = forwardingContext.getSchedulesForUserGroupId(getGroupId());
            setSchedules(schedules);
        }

        if (group != null) {
            return;
        }

        if (getFirstRun()) {
            setTab(CONFIGURE);
            setFirstRun(false);
        }

        group = getSettingDao().getGroup(getGroupId());
        setGroup(group);
        Setting settings = group.inherhitSettingsForEditing(getBean());
        setSettings(settings);

        if (null != getTab() && getTab().equals(TIMEZONE_TAB)) {
            // Init. the timezone dropdown menu.
            List<String> timezoneList = getTimeManager().getAvailableTimezones();

            // Sort list alphanumerically.
            Collections.sort(timezoneList, new AlphanumComparator());
            StringPropertySelectionModel model = new StringPropertySelectionModel(
                    timezoneList.toArray(new String[timezoneList.size()]));
            setTimezoneTypeModel(model);
            if (!event_.getRequestCycle().isRewinding()) {
                String utz = getGroup().getSettingValue(TIMEZONE_SETTING);
                if (utz != null) {
                    setTimezoneType(utz);
                }
            }

        }

        if (getFirstRun() || (null != getTab() && getParentSettingName() == null)
                || (null != getTab() && !getIsTabsSelected())) {
            setParentSetting(null);
            setParentSettingName(null);
            setIsTabsSelected(true);
            if (getTab().equals(SPEEDDIAL)) {
                setSpeedDialGroup(getSpeedDialManager().getSpeedDialForGroupId(getGroupId()));
            }
        } else {
            setTab(null);
            Setting parent = settings.getSetting(getParentSettingName());
            setParentSetting(parent);
            setIsTabsSelected(false);
        }
        setHotellingSetting(getSettings().getSetting("hotelling/enable"));
    }

    public IPage addSchedule(IRequestCycle cycle) {
        EditSchedule page = (EditSchedule) cycle.getPage(EditSchedule.PAGE);
        page.setUserId(null);
        page.setUserGroup(getSettingDao().getGroup(getGroupId()));
        page.newSchedule("usrGroup_sch", PAGE);
        return page;
    }

    public IPage editSchedulesGroup(IRequestCycle cycle, Integer scheduleId) {
        EditSchedule page = (EditSchedule) cycle.getPage(EditSchedule.PAGE);
        page.editSchedule(scheduleId, PAGE);
        return page;
    }

    public void editSchedule() {
        setParentSettingName(SCHEDULES);
    }

    public boolean isScheduleTabActive() {
        if (getParentSettingName() != null) {
            return getParentSettingName().equalsIgnoreCase(SCHEDULES);
        }
        return false;
    }

    public void editConferenceSettings() {
        setParentSettingName(CONFERENCE);
    }

    public boolean isConferenceTabActive() {
        return (CONFERENCE.equalsIgnoreCase(getParentSettingName()));
    }

    public void editExtContactSettings() {
        setParentSettingName(EXTCONTACT);
    }

    public boolean isExtContactTabActive() {
        return (EXTCONTACT.equalsIgnoreCase(getParentSettingName()));
    }

    public void onSpeedDialSubmit() {
        // XCF-1435 - Unless attempting to save data (e.g. onApply and the like)
        // clear all form errors
        // A.) user is probably not done and errors are disconcerting
        // B.) tapestry rewrites form values that are invalid on the button move operations
        // NOTE: This relies on the fact the the form listener is called BEFORE AND IN ADDITION TO
        // the button listener.
        if (!isValidationEnabled()) {
            TapestryUtils.getValidator(this).clearErrors();
        }
    }

    public void onSpeedDialApply() {
        setValidationEnabled(true);
        if (TapestryUtils.isValid(this)) {
            getSpeedDialManager().saveSpeedDialGroup(getSpeedDialGroup());
        }
    }

    public void onSpeedDialUpdatePhones() {
        setValidationEnabled(true);
        if (TapestryUtils.isValid(this)) {
            onSpeedDialApply();
            updatePhones();
        }
    }

    public void editVoicemailSettings() {
        setParentSettingName(VOICEMAIL);
    }

    public boolean isVoicemailTabActive() {
        return (VOICEMAIL.equalsIgnoreCase(getParentSettingName()));
    }

    public Setting getVoicemailSettings() {
        return getSettings().getSetting(VOICEMAIL);
    }

    public void onMohUpdatePhones() {
        if (TapestryUtils.isValid(this)) {
            apply();
            updatePhones();
        }
    }

    public Setting getMohSettings() {
        return getSettings().getSetting(MOH);
    }

    private void updatePhones() {
        Collection<Integer> ids = getPhoneContext().getPhoneIdsByUserGroupId(getGroupId());
        getProfileManager().generateProfiles(ids, true, null);
    }

    public void onApplyTimeZone() {
        getGroup().setSettingValue(TIMEZONE_SETTING, getTimezoneType());
        apply();
    }

    public String getGroupsToHide() {
        List<String> names = new LinkedList<String>();
        names.add(VOICEMAIL);
        names.add(MOH);
        if (!getFeatureManager().isFeatureEnabled(ImBot.FEATURE)) {
            names.add("im_notification");
        }
        if (!isVoicemailEnabled()) {
            names.add("personal-attendant");
        }
        names.add(TIMEZONE_TAB);
        names.add(HOTELLING_TAB);
        return StringUtils.join(names, SEPARATOR);
    }

    public String getSettingsToHide() {
        List<String> names = new LinkedList<String>();
        if (!getFeatureManager().isFeatureEnabled(ImBot.FEATURE)) {
            names.add("add-pa-to-group");
        }
        if (!isVoicemailEnabled()) {
            names.add("leaveMsgBeginIM, leaveMsgEndIM");
        }
        return StringUtils.join(names, SEPARATOR);
    }

    public boolean isRenderBranchOption() {
        return getGroup().getBranch() != null;
    }

    public boolean isRenderTimeZoneDropDown() {
        return getGroup().getBranch() == null
                || !(Boolean) getSettings().getSetting("timezone/useBranchTimezone").getTypedValue();
    }

    public boolean getSubscribePresenceGroupDisabled() {
        Setting groupSettings = getGroup().inherhitSettingsForEditing(getBean());
        Setting setting = groupSettings.getSetting(PermissionName.SUBSCRIBE_TO_PRESENCE.getPath());
        if (setting == null) {
            throw new IllegalArgumentException("Setting " + PermissionName.SUBSCRIBE_TO_PRESENCE.getName()
                    + " does not exist in user setting model");
        }

        return !Permission.isEnabled(setting.getValue());
    }

    public void onChangeHotelling() {
        getGroup().setSettingValue(HOTELLING_SETTING, getHotellingSetting().getValue());
        apply();
        getHotellingManager().generate(getGroup());
    }
}
