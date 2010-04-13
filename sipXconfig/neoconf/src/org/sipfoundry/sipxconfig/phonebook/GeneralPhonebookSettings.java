/*
 *
 *
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.phonebook;

import org.sipfoundry.sipxconfig.setting.BeanWithSettings;
import org.sipfoundry.sipxconfig.setting.Setting;

public class GeneralPhonebookSettings extends BeanWithSettings {

    private static final String EVERYONE = "application/everyone";

    @Override
    protected Setting loadSettings() {
        Setting settings = getModelFilesContext().loadModelFile("phonebook-settings.xml", "commserver");
        return settings;
    }

    public boolean isEveryoneEnabled() {
        Setting everyoneSetting = getSettings().getSetting(EVERYONE);
        return (Boolean) everyoneSetting.getTypedValue();
    }

    public void setEveryoneEnabled(boolean enabled) {
        Setting everyoneSetting = getSettings().getSetting(EVERYONE);
        everyoneSetting.setTypedValue(enabled);
    }
}
