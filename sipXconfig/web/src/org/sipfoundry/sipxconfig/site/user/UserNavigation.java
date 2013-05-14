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
import java.util.Collection;
import java.util.LinkedList;
import java.util.List;

import org.apache.commons.lang.StringUtils;
import org.apache.tapestry.IPage;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.InjectPage;
import org.apache.tapestry.annotations.Parameter;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.imbot.ImBot;
import org.sipfoundry.sipxconfig.ivr.Ivr;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.site.admin.time.EditTimeZonePage;
import org.sipfoundry.sipxconfig.site.common.BeanNavigation;
import org.sipfoundry.sipxconfig.site.moh.MusicOnHoldPage;
import org.sipfoundry.sipxconfig.site.speeddial.SpeedDialPage;
import org.sipfoundry.sipxconfig.site.user_portal.ExtendedUserInfo;
import org.sipfoundry.sipxconfig.site.user_portal.UserCallForwarding;
import org.sipfoundry.sipxconfig.site.user_portal.UserSchedules;
import org.sipfoundry.sipxconfig.site.vm.MailboxPreferencesPage;
import org.sipfoundry.sipxconfig.vm.MailboxManager;

public abstract class UserNavigation extends BeanNavigation {

    private static final String PERSONAL_ATTENDANT = "personal-attendant";

    @Parameter(required = false, defaultValue = "true")
    public abstract void setRenderCondition(boolean renderCondition);

    @InjectObject(value = "spring:coreContext")
    public abstract CoreContext getCoreContext();

    @InjectObject("spring:featureManager")
    public abstract FeatureManager getFeatureManager();

    @InjectPage(value = SpeedDialPage.PAGE)
    public abstract SpeedDialPage getSpeedDialPage();

    @InjectPage(value = UserCallForwarding.PAGE)
    public abstract UserCallForwarding getUserCallForwardingPage();

    @InjectPage(value = UserSchedules.PAGE)
    public abstract UserSchedules getUserSchedulesPage();

    @InjectPage(value = EditUser.PAGE)
    public abstract EditUser getEditUserPage();

    @InjectPage(value = UserSettings.PAGE)
    public abstract UserSettings getUserSettingsPage();

    @InjectPage(value = EditPersonalAttendant.PAGE)
    public abstract EditPersonalAttendant getEditPersonalAttendantPage();

    @InjectPage(value = UserPhones.PAGE)
    public abstract UserPhones getUserPhonesPage();

    @InjectPage(value = UserRegistrations.PAGE)
    public abstract UserRegistrations getUserRegistrationsPage();

    @InjectPage(value = UserConferences.PAGE)
    public abstract UserConferences getUserConferencesPage();

    @InjectPage(value = ExtendedUserInfo.PAGE)
    public abstract ExtendedUserInfo getExtendedUserInfoPage();

    @InjectPage(value = MailboxPreferencesPage.PAGE)
    public abstract MailboxPreferencesPage getMailboxPreferencesPage();

    @InjectPage(value = MusicOnHoldPage.PAGE)
    public abstract MusicOnHoldPage getMusicOnHoldPage();

    @InjectObject(value = "spring:mailboxManager")
    public abstract MailboxManager getMailboxManager();

    @InjectPage(value = EditTimeZonePage.PAGE)
    public abstract EditTimeZonePage getEditTimeZonePage();

    public IPage editCallForwarding(Integer userId) {
        UserCallForwarding page = getUserCallForwardingPage();
        page.setUserId(userId);
        page.setReturnPage(ManageUsers.PAGE);
        return page;
    }

    public IPage editSchedules(Integer userId) {
        UserSchedules page = getUserSchedulesPage();
        page.setUserId(userId);
        page.setReturnPage(ManageUsers.PAGE);
        return page;
    }

    public IPage editSpeedDial(Integer userId) {
        SpeedDialPage page = getSpeedDialPage();
        page.setUserId(userId);
        page.setReturnPage(ManageUsers.PAGE);
        return page;
    }

    public IPage editUser(Integer userId) {
        EditUser page = getEditUserPage();
        page.setUserId(userId);
        return page;
    }

    public IPage editPersonalAttendant(Integer userId) {
        EditPersonalAttendant page = getEditPersonalAttendantPage();
        page.setUserId(userId);
        page.setReturnPage(ManageUsers.PAGE);
        return page;
    }

    public IPage editSettings(Integer beanId, String path) {
        UserSettings page = getUserSettingsPage();
        page.setUserId(beanId);
        page.setParentSettingName(path);
        page.setReturnPage(ManageUsers.PAGE);
        return page;
    }

    public IPage editUserPhones(Integer userId) {
        UserPhones page = getUserPhonesPage();
        page.setUserId(userId);
        page.setReturnPage(ManageUsers.PAGE);
        return page;
    }

    public IPage viewUserRegistrations(Integer userId) {
        UserRegistrations page = getUserRegistrationsPage();
        page.setUserId(userId);
        page.setReturnPage(ManageUsers.PAGE);
        return page;
    }

    public IPage viewUserConferences(Integer userId) {
        UserConferences page = getUserConferencesPage();
        page.setUserId(userId);
        page.setReturnPage(ManageUsers.PAGE);
        return page;
    }

    public IPage editExtendedUserInfo(Integer userId) {
        ExtendedUserInfo page = getExtendedUserInfoPage();
        page.setUserId(userId);
        page.setReturnPage(ManageUsers.PAGE);
        return page;
    }

    public IPage viewMailboxPreferences(Integer userId) {
        MailboxPreferencesPage page = getMailboxPreferencesPage();
        page.setUserId(userId);
        page.setReturnPage(ManageUsers.PAGE);
        return page;
    }

    public IPage editMusicOnHold(Integer userId) {
        MusicOnHoldPage page = getMusicOnHoldPage();
        page.setUserId(userId);
        page.setReturnPage(ManageUsers.PAGE);
        return page;
    }

    public IPage editTimeZone(Integer userId) {
        EditTimeZonePage page = getEditTimeZonePage();
        page.setUserId(userId);
        page.setReturnPage(EditTimeZonePage.PAGE);
        return page;
    }

    public String getGroupsToHide() {
        List<String> names = new LinkedList<String>();
        names.add("voicemail");
        names.add(PERSONAL_ATTENDANT);
        names.add("callfwd");
        names.add("moh");
        names.add("timezone");
        if (!getFeatureManager().isFeatureEnabled(ImBot.FEATURE)) {
            names.add("im_notification");
        }
        return StringUtils.join(names, ",");
    }

    public boolean isVoicemailEnabled() {
        return (getFeatureManager().isFeatureEnabled(Ivr.FEATURE) ? true : false);
    }

    public boolean isConferencesTabActive() {
        return UserConferences.PAGE.equals(getPage().getPageName());
    }

    public boolean isIdentificationTabActive() {
        return EditUser.PAGE.equals(getPage().getPageName());
    }

    public boolean isPhonesTabActive() {
        return UserPhones.PAGE.equals(getPage().getPageName());
    }

    public boolean isCallForwardingTabActive() {
        return UserCallForwarding.PAGE.equals(getPage().getPageName());
    }

    public boolean isUserSchedulesTabActive() {
        return UserSchedules.PAGE.equals(getPage().getPageName());
    }

    public boolean isSpeedDialTabActive() {
        return SpeedDialPage.PAGE.equals(getPage().getPageName());
    }

    public boolean isPersonalAttendantTabActive() {
        return EditPersonalAttendant.PAGE.equals(getPage().getPageName());
    }

    public boolean isRegistrationsTabActive() {
        return UserRegistrations.PAGE.equals(getPage().getPageName());
    }

    public boolean isExtendedUserInfoTabActive() {
        return ExtendedUserInfo.PAGE.equals(getPage().getPageName());
    }

    public boolean isMailboxPreferencesTabActive() {
        return MailboxPreferencesPage.PAGE.equals(getPage().getPageName());
    }

    public boolean isMusicOnHoldTabActive() {
        return MusicOnHoldPage.PAGE.equals(getPage().getPageName());
    }

    public boolean isTimeZoneTabActive() {
        return EditTimeZonePage.PAGE.equals(getPage().getPageName());
    }

    public Collection<Setting> getNavigationGroups() {
        Setting settings = getBean().getSettings();
        return getUserNavigationGroups(settings);
    }

    /**
     * We need to flatten user settings so the permissions show up on a higher level. We convert
     * tree that looks like this: <code>
     * - permission
     * -- application permissions
     * -- call handling
     * - group 1
     * - group 2
     * </code>
     *
     * Into tree that looks like this: <code>
     * - application permissions
     * - call handling
     * - group 1
     * - group 2
     * </code>
     *
     * Additionally, the personal-attendant group is hidden as this tab is implemented as its own
     * page
     */
    public static Collection<Setting> getUserNavigationGroups(Setting settings) {
        Collection<Setting> result = new ArrayList<Setting>();
        for (Setting group : settings.getValues()) {
            if (group.getParent() != settings) {
                // only first level groups are interesting
                continue;
            }
            if (group.getName().equals("permission")) {
                result.addAll(group.getValues());
            } else if (group.getName().equals(PERSONAL_ATTENDANT)) {
                // skip this group
                continue;
            } else {
                result.add(group);
            }
        }

        return result;
    }
}
