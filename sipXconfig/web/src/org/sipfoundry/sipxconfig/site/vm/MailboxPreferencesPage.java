/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.vm;

import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.site.user_portal.UserBasePage;
import org.sipfoundry.sipxconfig.vm.Mailbox;
import org.sipfoundry.sipxconfig.vm.MailboxManager;
import org.sipfoundry.sipxconfig.vm.MailboxPreferences;

public abstract class MailboxPreferencesPage extends UserBasePage {
    public static final String PAGE = "vm/MailboxPreferencesPage";
    private static final String HOST_SETTING = "unified-messaging/host";
    private static final String PORT_SETTING = "unified-messaging/port";
    private static final String TLS_SETTING = "unified-messaging/tls";

    @InjectObject(value = "spring:mailboxManager")
    public abstract MailboxManager getMailboxManager();

    public abstract MailboxPreferences getMailboxPreferences();

    public abstract void setMailboxPreferences(MailboxPreferences preferences);

    @Persist
    public abstract Setting getSettings();

    public abstract void setSettings(Setting settings);

    @Persist("client")
    public abstract boolean isAdvanced();

    @Override
    public void pageBeginRender(PageEvent event) {
        super.pageBeginRender(event);

        if (getSettings() == null) {
            setSettings(getUser().getSettings().getSetting("unified-messaging"));
        }

        User user = getUser();

        MailboxPreferences preferences = getMailboxPreferences();
        MailboxManager mmgr = getMailboxManager();

        if (preferences == null && mmgr.isEnabled()) {
            Mailbox mailbox = mmgr.getMailbox(user.getUserName());
            preferences = mmgr.loadMailboxPreferences(mailbox);
            preferences.setUser(user);
            setMailboxPreferences(preferences);
        }
    }

    public void onSubmit() {
    }

    public void onApply() {
        MailboxManager mmgr = getMailboxManager();
        User user = getUser();
        user.setSettingValue(HOST_SETTING, getSettings().getSetting("host").getValue());
        user.setSettingValue(PORT_SETTING, getSettings().getSetting("port").getValue());
        user.setSettingValue(TLS_SETTING, getSettings().getSetting("tls").getValue());
        getCoreContext().saveUser(user);
        if (mmgr.isEnabled()) {
            MailboxPreferences preferences = getMailboxPreferences();
            preferences.setEmailServerHost(user.getSettingValue(HOST_SETTING));
            preferences.setEmailServerPort(user.getSettingValue(PORT_SETTING));
            preferences.setEmailServerUseTLS((Boolean) user.getSettingTypedValue(TLS_SETTING));

            Mailbox mailbox = mmgr.getMailbox(user.getUserName());
            mmgr.saveMailboxPreferences(mailbox, preferences);
        }
    }
}
