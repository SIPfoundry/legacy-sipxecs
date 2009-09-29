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

import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.callback.PageCallback;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.setting.SettingDao;
import org.sipfoundry.sipxconfig.site.setting.EditGroup;
import org.sipfoundry.sipxconfig.speeddial.SpeedDialManager;
import org.sipfoundry.sipxconfig.vm.Mailbox;
import org.sipfoundry.sipxconfig.vm.MailboxManager;
import org.sipfoundry.sipxconfig.vm.MailboxPreferences;

public abstract class EditUser extends PageWithCallback implements PageBeginRenderListener {

    public static final String PAGE = "user/EditUser";

    @Bean
    public abstract SipxValidationDelegate getValidator();

    @InjectObject("spring:speedDialManager")
    public abstract SpeedDialManager getSpeedDialManager();

    @InjectObject("spring:coreContext")
    public abstract CoreContext getCoreContext();

    @InjectObject("spring:settingDao")
    public abstract SettingDao getSettingDao();

    @InjectObject("spring:mailboxManager")
    public abstract MailboxManager getMailboxManager();

    @Persist
    public abstract Integer getUserId();

    public abstract void setUserId(Integer userId);

    public abstract User getUser();

    public abstract void setUser(User user);

    public void commit() {
        if (!TapestryUtils.isValid(this)) {
            return;
        }
        User user = getUser();
        EditGroup.saveGroups(getSettingDao(), user.getGroups());
        boolean newUsername = getCoreContext().saveUser(user);

        MailboxManager mmgr = getMailboxManager();
        if (mmgr.isEnabled()) {
            if (newUsername) {
                mmgr.deleteMailbox(user.getUserName());
            }

            Mailbox mailbox = mmgr.getMailbox(user.getUserName());
            MailboxPreferences preferences = mmgr.loadMailboxPreferences(mailbox);
            preferences.setEmailServerHost(user.getSettingValue("unified-messaging/host"));
            preferences.setEmailServerPort(user.getSettingValue("unified-messaging/port"));
            preferences.setEmailServerUseTLS((Boolean) user.getSettingTypedValue("unified-messaging/tls"));

            mmgr.saveMailboxPreferences(mailbox, preferences);
        }

        if (newUsername) {
            // FIXME: this should be done automatically by speed dial manager
            getSpeedDialManager().activateResourceList();
        }
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
}
