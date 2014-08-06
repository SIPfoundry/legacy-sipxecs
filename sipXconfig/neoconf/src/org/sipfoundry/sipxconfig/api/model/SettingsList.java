/**
 * Copyright (c) 2014 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.api.model;

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;
import java.util.Locale;

import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlRootElement;

import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingArray;

@XmlRootElement(name = "Settings")
public class SettingsList {

    private List<SettingBean> m_settings;
    public void setSettings(List<SettingBean> settings) {
        m_settings = settings;
    }

    @XmlElement(name = "Setting")
    public List<SettingBean> getSettings() {
        if (m_settings == null) {
            m_settings = new ArrayList<SettingBean>();
        }
        return m_settings;
    }

    public static SettingsList convertSettingsList(Setting settings, Locale locale) {
        List<SettingBean> settingsList = new ArrayList<SettingBean>();

        addSettingToList(settingsList, settings, locale);

        SettingsList list = new SettingsList();
        list.setSettings(settingsList);
        return list;
    }

    private static void addSettingToList(List<SettingBean> settingsList, Setting settings, Locale locale) {
        Collection<Setting> settingsToIterate = null;
        if (settings instanceof SettingArray) {
            SettingArray settingsArray = (SettingArray) settings;
            String[] settingNames = settingsArray.getSettingNames();
            settingsToIterate = new ArrayList<Setting>();

            for (int i = 0; i < settingsArray.getSize(); i++) {
                for (int j = 0; j < settingNames.length; j++) {
                    settingsToIterate.add(settingsArray.getSetting(i, settingNames[j]));
                }
            }
        } else {
            settingsToIterate = settings.getValues();
        }

        addSettingsToList(settingsList, settingsToIterate, locale);
    }

    private static void addSettingsToList(List<SettingBean> settingsList, Collection<Setting> settings, Locale locale) {
        for (Setting setting : settings) {
            if (!setting.isHidden()) {
                if (setting.isLeaf()) {
                    settingsList.add(SettingBean.convertSetting(setting, locale));
                } else {
                    addSettingToList(settingsList, setting, locale);
                }
            }
        }

    }

}
