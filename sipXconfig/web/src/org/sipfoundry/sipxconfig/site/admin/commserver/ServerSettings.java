/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site.admin.commserver;

import org.apache.tapestry.IPage;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InitialValue;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.InjectPage;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.admin.commserver.Server;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.setting.Setting;

/**
 * Page presenting sipX server settings: in most cases obtained by reading values from
 * configuration files in /etc/sipxpbx directory
 */
public abstract class ServerSettings extends BasePage implements PageBeginRenderListener {
    public static final String PAGE = "admin/commserver/ServerSettings";

    @InjectObject(value = "spring:sipxServer")
    public abstract Server getServer();

    @Bean
    public abstract SipxValidationDelegate getValidator();

    @Persist(value = "session")
    @InitialValue(value = "literal:voice-mail")
    public abstract String getParentSettingName();

    @InjectPage(value = RestartReminder.PAGE)
    public abstract RestartReminder getRestartReminderPage();

    public abstract void setParentSettingName(String name);

    public abstract Setting getParentSetting();

    public abstract void setParentSetting(Setting parent);

    public void pageBeginRender(PageEvent event_) {
        if (getParentSetting() != null) {
            return;
        }
        Server server = getServer();
        Setting root = server.getSettings();
        Setting parent = root.getSetting(getParentSettingName());
        setParentSetting(parent);
    }

    @SuppressWarnings("unused")
    public void editSettings(Integer serverId_, String settingPath) {
        setParentSettingName(settingPath);
    }

    public IPage apply() {
        getServer().applySettings();
        RestartReminder restartPage = getRestartReminderPage();
        restartPage.setNextPage(PAGE);
        return restartPage;
    }

    public void cancel() {
        getServer().resetSettings();
    }
}
