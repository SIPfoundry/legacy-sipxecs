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

import java.util.Set;

import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.callback.PageCallback;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.SettingDao;
import org.sipfoundry.sipxconfig.site.user_portal.UserBasePage;
import org.sipfoundry.sipxconfig.speeddial.SpeedDialManager;
import org.sipfoundry.sipxconfig.vm.MailboxManager;

public abstract class EditUser extends UserBasePage implements PageBeginRenderListener {

    public static final String PAGE = "user/EditUser";

    @Bean
    public abstract SipxValidationDelegate getValidator();

    @InjectObject("spring:speedDialManager")
    public abstract SpeedDialManager getSpeedDialManager();

    @InjectObject("spring:settingDao")
    public abstract SettingDao getSettingDao();

    @InjectObject("spring:mailboxManager")
    public abstract MailboxManager getMailboxManager();

    public abstract User getUser();

    public abstract void setUser(User user);

    public void commit() {
        if (!TapestryUtils.isValid(this)) {
            return;
        }
        User user = getUser();
        saveGroups(getSettingDao(), user.getGroups());
        String oldUserName = getCoreContext().getOriginalUserName(user);
        String newUserName = user.getUserName();
        boolean userNameChanged = getCoreContext().saveUser(user);

        MailboxManager mmgr = getMailboxManager();
        if (mmgr.isEnabled() && userNameChanged) {
            mmgr.renameMailbox(oldUserName, newUserName);
        }

//        if (userNameChanged) {
//            // FIXME: this should be done automatically by speed dial manager
//            getSpeedDialManager().activateResourceList();
//        }
    }

    public void pageBeginRender(PageEvent event_) {
        User user = getUser();
        if (user == null) {
            user = getCoreContext().loadUser(getUserId());
            setUser(user);
        }

        // If no callback has been given, then navigate back to Manage Users on OK/Cancel
        if (getCallback() == null) {
            setCallback(new PageCallback(ManageUsers.PAGE));
        }
    }

    public static void saveGroups(SettingDao dao, Set<Group> groups) {
        for (Group group : groups) {
            if (group.isNew()) {
                dao.saveGroup(group);
            }
        }
    }

}
