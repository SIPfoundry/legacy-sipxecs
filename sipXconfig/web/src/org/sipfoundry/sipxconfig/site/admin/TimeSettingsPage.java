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

import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InitialValue;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.sipfoundry.sipxconfig.admin.time.TimeManager;
import org.sipfoundry.sipxconfig.components.MapSelectionModel;
import org.sipfoundry.sipxconfig.components.SipxBasePage;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;

public abstract class TimeSettingsPage extends SipxBasePage {
    @InjectObject(value = "spring:timeManager")
    public abstract TimeManager getTimeManager();

    @Bean
    public abstract SipxValidationDelegate getValidator();

    @Persist(value = "client")
    @InitialValue(value = "ognl:timeManager.systemTimeSettingType")
    public abstract Integer getTimeSettingType();

    @Persist
    @InitialValue(value = "literal:timeAndDate")
    public abstract String getTab();

    public IPropertySelectionModel getTimeSettingTypeModel() {
        Map<Integer, String> map = new HashMap<Integer, String>();
        map.put(new Integer(0), getMessages().getMessage("label.NTPTime"));
        map.put(new Integer(1), getMessages().getMessage("label.manualTime"));
        return new MapSelectionModel(map);
    }

    public boolean getManualSetting() {
        return getTimeSettingType().equals(new Integer(1));
    }
}
