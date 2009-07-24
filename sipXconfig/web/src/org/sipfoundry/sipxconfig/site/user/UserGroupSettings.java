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

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.admin.forwarding.ForwardingContext;
import org.sipfoundry.sipxconfig.admin.forwarding.UserGroupSchedule;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.conference.ConferenceBridgeContext;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.site.setting.EditGroup;
import org.sipfoundry.sipxconfig.site.setting.EditSchedule;
import org.sipfoundry.sipxconfig.site.setting.GroupSettings;
import org.sipfoundry.sipxconfig.vm.Mailbox;
import org.sipfoundry.sipxconfig.vm.MailboxManager;
import org.sipfoundry.sipxconfig.vm.MailboxPreferences;

public abstract class UserGroupSettings extends GroupSettings {
    @SuppressWarnings("hiding")
    public static final String PAGE = "user/UserGroupSettings";

    private static final String SCHEDULES = "Schedules";
    private static final String CONFERENCE = "conference";

    private static final String HOST_SETTING = "unified-messaging/host";
    private static final String PORT_SETTING = "unified-messaging/port";
    private static final String TLS_SETTING = "unified-messaging/tls";

    @InjectObject(value = "spring:forwardingContext")
    public abstract ForwardingContext getForwardingContext();

    @InjectObject(value = "spring:conferenceBridgeContext")
    public abstract ConferenceBridgeContext getConferenceBridgeContext();

    public abstract void setSchedules(List<UserGroupSchedule> schedules);

    public abstract List<UserGroupSchedule> getSchedules();

    public abstract boolean getChanged();

    @InjectObject(value = "spring:mailboxManager")
    public abstract MailboxManager getMailboxManager();

    @InjectObject(value = "spring:coreContext")
    public abstract CoreContext getCoreContext();

    @Override
    public IPage editGroupName(IRequestCycle cycle) {
        EditGroup page = (EditGroup) cycle.getPage(EditGroup.PAGE);
        page.editGroup(getGroupId(), PAGE);
        return page;
    }

    @Override
    public void pageBeginRender(PageEvent event_) {
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

        group = getSettingDao().getGroup(getGroupId());
        setGroup(group);
        Setting settings = group.inherhitSettingsForEditing(getBean());
        setSettings(settings);

        if (getParentSettingName() == null) {
            setParentSettingName(SCHEDULES);
        }

        if (!getParentSettingName().equalsIgnoreCase(SCHEDULES)) {
            Setting parent = settings.getSetting(getParentSettingName());
            setParentSetting(parent);
        }

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

    @Override
    public void apply() {
        super.apply();
        Collection<User> users = getCoreContext().getGroupMembers(getGroup());
        String host = getSettings().getSetting(HOST_SETTING).getValue();
        String port = getSettings().getSetting(PORT_SETTING).getValue();
        String tls = getSettings().getSetting(TLS_SETTING).getValue();
        for (User user : users) {
            List<Group> groupsForUser = user.getGroupsAsList();
            List<Group> unifiedMessagingGroups = new ArrayList<Group>();
            for (Group groupForUser : groupsForUser) {
                String groupHost = groupForUser.getSettingValue(HOST_SETTING);
                if (groupHost != null) {
                    unifiedMessagingGroups.add(groupForUser);
                }
            }
            Group highestWeightGroup = Group.selectGroupWithHighestWeight(unifiedMessagingGroups);
            if (highestWeightGroup == null || getGroup().equals(highestWeightGroup)) {
                Mailbox mailbox = getMailboxManager().getMailbox(user.getName());
                MailboxPreferences preferences = getMailboxManager().loadMailboxPreferences(mailbox);
                preferences.setEmailServerHost(host);
                preferences.setEmailServerPort(port);
                if (tls != null && tls.equals("1")) {
                    preferences.setEmailServerUseTLS(true);
                } else {
                    preferences.setEmailServerUseTLS(false);
                }
                getMailboxManager().saveMailboxPreferences(mailbox, preferences);
            }
        }
    }
}
