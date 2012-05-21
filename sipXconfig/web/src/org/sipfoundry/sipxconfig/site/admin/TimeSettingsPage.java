/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.admin;

import java.util.HashMap;
import java.util.Map;

import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InitialValue;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.sipfoundry.sipxconfig.components.MapSelectionModel;
import org.sipfoundry.sipxconfig.components.SipxBasePage;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.time.NtpManager;
import org.sipfoundry.sipxconfig.time.NtpSettings;

public abstract class TimeSettingsPage extends SipxBasePage implements PageBeginRenderListener {
    @InjectObject(value = "spring:ntpManager")
    public abstract NtpManager getTimeManager();

    public abstract NtpSettings getSettings();

    public abstract void setSettings(NtpSettings settings);

    @Bean
    public abstract SipxValidationDelegate getValidator();

    @Persist
    @InitialValue(value = "literal:timeAndDate")
    public abstract String getTab();

    @Override
    public void pageBeginRender(PageEvent arg0) {
        if (getSettings() == null) {
            setSettings(getTimeManager().getSettings());
        }
    }

    public void apply(IRequestCycle cycle) {
        getTimeManager().saveSettings(getSettings());
    }

    public IPropertySelectionModel getTimeSettingTypeModel() {
        Map<Integer, String> map = new HashMap<Integer, String>();
        map.put(new Integer(0), getMessages().getMessage("label.NTPTime"));
        map.put(new Integer(1), getMessages().getMessage("label.manualTime"));
        return new MapSelectionModel(map);
    }

}
