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

import org.apache.tapestry.IPage;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.InjectPage;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.site.common.BeanNavigation;
import org.sipfoundry.sipxconfig.site.speeddial.SpeedDialPage;
import org.sipfoundry.sipxconfig.site.user_portal.UserCallForwarding;
import org.sipfoundry.sipxconfig.site.user_portal.UserSchedules;

public abstract class UserNavigation extends BeanNavigation<User> {

    @InjectObject(value = "spring:coreContext")
    public abstract CoreContext getCoreContext();

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

    @InjectPage(value = SupervisorPermission.PAGE)
    public abstract SupervisorPermission getSupervisorPermissionPage();

    @InjectPage(value = UserPhones.PAGE)
    public abstract UserPhones getUserPhonesPage();

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
        return page;
    }

    public IPage editSupervisorPermission(Integer userId) {
        SupervisorPermission page = getSupervisorPermissionPage();
        page.setUserId(userId);
        page.setReturnPage(ManageUsers.PAGE);
        return page;
    }

    public IPage editUserPhones(Integer userId) {
        UserPhones page = getUserPhonesPage();
        page.setUserId(userId);
        page.setReturnPage(ManageUsers.PAGE);
        return page;
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

    public boolean isSupervisorTabActive() {
        return SupervisorPermission.PAGE.equals(getPage().getPageName());
    }
    
    public boolean isPersonalAttendantTabActive() {
        return EditPersonalAttendant.PAGE.equals(getPage().getPageName());
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
     * Additionally, the personal-attendant group is hidden as this tab is implemented
     * as its own page
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
            } else if (group.getName().equals("personal-attendant")) {
                // skip this group
                continue;
            } else {
                result.add(group);
            }
        }

        return result;
    }
}
