/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site;

import java.util.Collection;
import java.util.HashMap;
import java.util.Map;

import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.components.SipxBasePage;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.setting.ModelBuilder;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingImpl;
import org.sipfoundry.sipxconfig.setting.SettingModel;
import org.sipfoundry.sipxconfig.setting.SettingSet;
import org.sipfoundry.sipxconfig.setting.SettingValue;
import org.sipfoundry.sipxconfig.setting.SettingValueImpl;
import org.sipfoundry.sipxconfig.setting.type.IntegerSetting;
import org.sipfoundry.sipxconfig.site.setting.SettingsIron;

public abstract class UiFieldsTestPage extends SipxBasePage implements PageBeginRenderListener {

    @InjectObject(value = "spring:modelBuilder")
    public abstract ModelBuilder getModelBuilder();

    @Bean
    public abstract SipxValidationDelegate getValidator();

    @Persist
    public abstract Setting getSettings();
    public abstract void setSettings(Setting settings);

    @Persist
    public abstract Collection<Setting> getFlatSettings();
    public abstract void setFlatSettings(Collection<Setting> settings);

    public abstract Setting getCurrentSetting();

    public void pageBeginRender(PageEvent event) {
        if (getSettings() == null) {
            Setting settings = new SettingSet("ui-fields-test");
            SimpleSettingModel settingModel = new SimpleSettingModel();

            SettingImpl minMaxRequiredSetting = new SettingImpl("int-min-max-required");
            IntegerSetting minMaxSettingType = new IntegerSetting();
            minMaxSettingType.setMin(1);
            minMaxSettingType.setMax(100);
            minMaxSettingType.setRequired(true);
            minMaxRequiredSetting.setType(minMaxSettingType);
            minMaxRequiredSetting.setModel(settingModel);
            settingModel.setDefaultSettingValue(minMaxRequiredSetting, new SettingValueImpl("25"));
            minMaxRequiredSetting.setValue("16");
            settings.addSetting(minMaxRequiredSetting);

            SettingImpl minMaxNotRequiredSetting = new SettingImpl("int-min-max-not-required");
            IntegerSetting minMaxNotRequiredSettingType = new IntegerSetting();
            minMaxNotRequiredSettingType.setMin(1);
            minMaxNotRequiredSettingType.setMax(100);
            minMaxNotRequiredSettingType.setRequired(false);
            minMaxNotRequiredSetting.setType(minMaxNotRequiredSettingType);
            minMaxNotRequiredSetting.setModel(settingModel);
            settings.addSetting(minMaxNotRequiredSetting);

            SettingImpl noConstraintsSetting = new SettingImpl("no-constraints");
            IntegerSetting noConstraintsSettingType = new IntegerSetting();
            noConstraintsSetting.setType(noConstraintsSettingType);
            settings.addSetting(noConstraintsSetting);

            setSettings(settings);

            SettingsIron settingsIron = new SettingsIron();
            settings.acceptVisitor(settingsIron);
            setFlatSettings(settingsIron.getFlat());

        }
    }

    public void ok() {

    }

    public void apply() {

    }

    public void cancel() {

    }

    private class SimpleSettingModel implements SettingModel {

        private Map<String, SettingValue> m_settingValueMap = new HashMap();
        private Map<String, SettingValue> m_settingDefaultValueMap = new HashMap();

        public SettingValue getDefaultSettingValue(Setting setting) {
            return m_settingDefaultValueMap.get(setting.getName());
        }

        public void setDefaultSettingValue(Setting setting, SettingValue settingValue) {
            m_settingValueMap.put(setting.getName(), settingValue);
        }

        public SettingValue getProfileName(Setting setting) {
            // TODO Auto-generated method stub
            return null;
        }

        public SettingValue getSettingValue(Setting setting) {
            return m_settingValueMap.get(setting.getName());
        }

        public void setSettingValue(Setting setting, String value) {
            m_settingValueMap.put(setting.getName(), new SettingValueImpl(value));
        }
    }
}
