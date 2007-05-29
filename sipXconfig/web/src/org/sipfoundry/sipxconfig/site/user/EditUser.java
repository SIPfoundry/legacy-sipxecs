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

import org.apache.tapestry.callback.PageCallback;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.setting.SettingDao;
import org.sipfoundry.sipxconfig.site.setting.EditGroup;
import org.sipfoundry.sipxconfig.vm.Mailbox;
import org.sipfoundry.sipxconfig.vm.MailboxManager;
import org.sipfoundry.sipxconfig.vm.MailboxPreferences;

public abstract class EditUser extends PageWithCallback implements PageBeginRenderListener {

    public static final String PAGE = "EditUser";

    public abstract CoreContext getCoreContext();

    public abstract SettingDao getSettingDao();

    public abstract Integer getUserId();

    public abstract void setUserId(Integer userId);

    public abstract User getUser();

    public abstract void setUser(User user);

    public abstract MailboxManager getMailboxManager();

    public abstract MailboxPreferences getMailboxPreferences();

    public abstract void setMailboxPreferences(MailboxPreferences preferences);

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
            mmgr.saveMailboxPreferences(mailbox, getMailboxPreferences());
        }
    }

    public void pageBeginRender(PageEvent event_) {
        User user = getUser();
        if (user == null) {
            user = getCoreContext().loadUser(getUserId());
            setUser(user);
        }

        MailboxPreferences preferences = getMailboxPreferences();
        MailboxManager mmgr = getMailboxManager();
        if (preferences == null && mmgr.isEnabled()) {
            Mailbox mailbox = mmgr.getMailbox(user.getUserName());
            preferences = mmgr.loadMailboxPreferences(mailbox);
            setMailboxPreferences(preferences);
        }

        // If no callback has been given, then navigate back to Manage Users on OK/Cancel
        if (getCallback() == null) {
            setCallback(new PageCallback(ManageUsers.PAGE));
        }
    }
}
