/**
 *
 * Copyright (c) 2013 Karel Electronics Corp. All rights reserved.
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
 *
 */

package org.sipfoundry.sipxconfig.site.admin.systemaudit;

import org.apache.log4j.Logger;
import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.sipfoundry.sipxconfig.admin.AdminSettings;
import org.sipfoundry.sipxconfig.admin.PasswordPolicy;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.components.LocalizationUtils;
import org.sipfoundry.sipxconfig.components.TapestryContext;
import org.sipfoundry.sipxconfig.conference.Conference;
import org.sipfoundry.sipxconfig.conference.ConferenceBridgeContext;
import org.sipfoundry.sipxconfig.dialplan.AutoAttendant;
import org.sipfoundry.sipxconfig.dialplan.AutoAttendantManager;
import org.sipfoundry.sipxconfig.gateway.GatewayContext;
import org.sipfoundry.sipxconfig.localization.LocalizationContext;
import org.sipfoundry.sipxconfig.parkorbit.ParkOrbit;
import org.sipfoundry.sipxconfig.parkorbit.ParkOrbitContext;
import org.sipfoundry.sipxconfig.permission.PermissionManager;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.setting.BeanWithSettings;
import org.sipfoundry.sipxconfig.setting.ModelFilesContext;
import org.sipfoundry.sipxconfig.setting.PersistableSettings;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.type.EnumSetting;
import org.sipfoundry.sipxconfig.setting.type.SettingType;
import org.sipfoundry.sipxconfig.site.admin.LocalizedLanguageMessages;
import org.sipfoundry.sipxconfig.site.setting.SettingEditor;
import org.sipfoundry.sipxconfig.systemaudit.ConfigChange;
import org.sipfoundry.sipxconfig.systemaudit.ConfigChangeType;
import org.sipfoundry.sipxconfig.systemaudit.ConfigChangeValue;
import org.springframework.context.MessageSource;

@ComponentClass(allowBody = false, allowInformalParameters = false)
public abstract class ConfigChangeValuesTable extends BaseComponent implements PageBeginRenderListener {

    public static final String COMPONENT = "ConfigChangeValuesTable";
    private static final Logger LOG = Logger.getLogger(ConfigChangeValuesTable.class);
    private static final String LOCALIZATION_PREFIX = "[";

    public abstract ConfigChangeValue getConfigChangeValue();

    @Parameter(required = true)
    public abstract Object getSource();

    @InjectObject(value = "spring:tapestry")
    public abstract TapestryContext getTapestry();

    @InjectObject("spring:modelFilesContext")
    public abstract ModelFilesContext getModelFilesContext();

    @InjectObject("spring:passwordPolicyImpl")
    public abstract PasswordPolicy getPasswordPolicy();

    @InjectObject("spring:permissionManager")
    public abstract PermissionManager getPermissionManager();

    @InjectObject("spring:phoneContext")
    public abstract PhoneContext getPhoneContext();

    @InjectObject("spring:gatewayContext")
    public abstract GatewayContext getGatewayContext();

    @InjectObject("spring:conferenceBridgeContext")
    public abstract ConferenceBridgeContext getConferenceBridgeContext();

    @InjectObject("spring:autoAttendantManager")
    public abstract AutoAttendantManager getAutoAttendantManager();

    @InjectObject("spring:parkOrbitContext")
    public abstract ParkOrbitContext getParkOrbitContext();

    @InjectObject("spring:localizationContext")
    public abstract LocalizationContext getLocalizationContext();

    @InjectObject("spring:localizedLanguageMessages")
    public abstract LocalizedLanguageMessages getLocalizedLanguageMessages();

    public String getPropertyName() {
        String propertyName = getConfigChangeValue().getPropertyName();
        // localize the normal way
        String localizedPropertyName = getMessages().getMessage(propertyName);
        if (!localizedPropertyName.startsWith(LOCALIZATION_PREFIX)) {
            return localizedPropertyName;
        }
        return getLocalizedPropertyName(propertyName);
    }

    public String getValueBefore() {
        return getLocalizedValue(getConfigChangeValue().getValueBefore());
    }

    public String getValueAfter() {
        return getLocalizedValue(getConfigChangeValue().getValueAfter());
    }

    private String getLocalizedValue(String value) {
        // if is null, nothing to localize
        if (value == null) {
            return value;
        }
        // localize the normal way
        String localizedValue = getMessages().getMessage(value);
        if (!localizedValue.startsWith(LOCALIZATION_PREFIX)) {
            return localizedValue;
        }
        // localize with propertyName as key prefix
        localizedValue = getMessages().getMessage(getConfigChangeValue().getPropertyName() + "." + value);
        if (!localizedValue.startsWith(LOCALIZATION_PREFIX)) {
            return localizedValue;
        }
        localizedValue = getLocalizeSettingValue(getConfigChangeValue().getPropertyName(), value);
        return localizedValue;
    }

    private String getLocalizeSettingValue(String propertyName, String value) {
        try {
            ConfigChange configChange = getConfigChangeValue().getConfigChange();
            Setting setting = null;
            if (configChange.getConfigChangeType() == ConfigChangeType.SETTINGS) {
                Class<?> clazz = Class.forName(configChange.getDetails());
                PersistableSettings settingObject = (PersistableSettings) clazz.newInstance();
                settingObject.setModelFilesContext(getModelFilesContext());
                if (settingObject instanceof AdminSettings) {
                    ((AdminSettings) settingObject).setPasswordPolicy(getPasswordPolicy());
                }
                setting = settingObject.getSettings().getSetting(propertyName);
            } else if (configChange.getConfigChangeType() == ConfigChangeType.USER) {
                User user = new User();
                user.setPermissionManager(getPermissionManager());
                setting = user.getSettings().getSetting(propertyName);
            } else if (configChange.getConfigChangeType() == ConfigChangeType.PHONE) {
                Phone phone = getPhoneContext().getPhoneBySerialNumber(configChange.getDetails());
                setting = phone.getSettings().getSetting(propertyName);
            } else if (configChange.getConfigChangeType() == ConfigChangeType.CONFERENCE) {
                Conference conference = getConferenceBridgeContext().findConferenceByName(configChange.getDetails());
                setting = conference.getSettings().getSetting(propertyName);
            } else if (configChange.getConfigChangeType() == ConfigChangeType.AUTO_ATTENDANT) {
                AutoAttendant aa = getAutoAttendantManager().getAutoAttendantBySystemName(configChange.getDetails());
                setting = aa.getSettings().getSetting(propertyName);
            } else if (configChange.getConfigChangeType() == ConfigChangeType.CALL_PARK) {
                ParkOrbit parkOrbit = getParkOrbitContext().loadParkOrbitByName(configChange.getDetails());
                setting = parkOrbit.getSettings().getSetting(propertyName);
            }
            if (setting != null) {
                String localizedValue = null;
                if (setting.getName().endsWith("language")) {
                    localizedValue = getLocalizedLanguageValue(value);
                }
                if (localizedValue != null) {
                    return localizedValue;
                }
                return getEnumLocalizedValue(setting, setting.getMessageSource(), value);
            }
        } catch (Exception e) {
            LOG.debug("Can't localize value: " + e.getMessage());
        }
        return value;
    }

    private String getLocalizedLanguageValue(String value) {
        String[] availableLanguages = getLocalizationContext().getInstalledLanguages();
        LocalizedLanguageMessages languageMessages = getLocalizedLanguageMessages();
        languageMessages.setAvailableLanguages(availableLanguages);
        String localizedValue = languageMessages.findMessage(value);
        if (localizedValue != null) {
            return localizedValue;
        } else {
            return value;
        }
    }

    private String getEnumLocalizedValue(Setting setting, MessageSource messageSource, String value) {
        SettingType type = setting.getType();
        if (!(type instanceof EnumSetting)) {
            return value;
        }
        EnumSetting enumType = (EnumSetting) type;
        IPropertySelectionModel model = null;
        model = (messageSource != null) ? SettingEditor.localizedModelForType(
                setting, enumType, messageSource, getPage().getLocale())
                : SettingEditor.enumModelForType(enumType);

        if (enumType.isPromptSelect()) {
            model = getTapestry().instructUserToSelect(model, getMessages());
        }
        for (int counter = 0; counter < model.getOptionCount(); counter++) {
            if (value.equals((String) model.translateValue(String.valueOf(counter)))) {
                return model.getLabel(counter);
            }
        }
        return value;
    }

    private String getLocalizedPropertyName(String message) {
        try {
            ConfigChange configChange = getConfigChangeValue().getConfigChange();
            if (configChange.getConfigChangeType() == ConfigChangeType.SETTINGS) {
                return getLocalizedSettingsMessage(configChange, message);
            } else if (configChange.getConfigChangeType() == ConfigChangeType.USER) {
                User user = new User();
                user.setPermissionManager(getPermissionManager());
                return getLocalizedBeanWithSettingsMessage(user, message);
            } else if (configChange.getConfigChangeType() == ConfigChangeType.PHONE) {
                Phone phone = getPhoneContext().getPhoneBySerialNumber(configChange.getDetails());
                return getLocalizedBeanWithSettingsMessage(phone, message);
            } else if (configChange.getConfigChangeType() == ConfigChangeType.CONFERENCE) {
                Conference conference = getConferenceBridgeContext().findConferenceByName(configChange.getDetails());
                return getLocalizedBeanWithSettingsMessage(conference, message);
            } else if (configChange.getConfigChangeType() == ConfigChangeType.AUTO_ATTENDANT) {
                AutoAttendant aa = getAutoAttendantManager().getAutoAttendantByName(configChange.getDetails());
                return getLocalizedBeanWithSettingsMessage(aa, message);
            } else if (configChange.getConfigChangeType() == ConfigChangeType.CALL_PARK) {
                ParkOrbit parkOrbit = getParkOrbitContext().loadParkOrbitByName(configChange.getDetails());
                return getLocalizedBeanWithSettingsMessage(parkOrbit, message);
            }
        } catch (Exception e) {
            LOG.debug("Can't localize string: " + e.getMessage());
        }
        return message;
    }

    private String getLocalizedBeanWithSettingsMessage(
            BeanWithSettings beanWithSettings, String message) throws Exception {
        Setting settings = beanWithSettings.getSettings();
        return LocalizationUtils.getSettingLabel(this,
                settings.getSetting(message));
    }

    /**
     *  Method used for extracting localizations from a Setting configChange
     */
    private String getLocalizedSettingsMessage(ConfigChange configChange, String message) throws Exception {
        Class<?> clazz = Class.forName(configChange.getDetails());
        PersistableSettings setting = (PersistableSettings) clazz.newInstance();
        setting.setModelFilesContext(getModelFilesContext());
        if (setting instanceof AdminSettings) {
            ((AdminSettings) setting).setPasswordPolicy(getPasswordPolicy());
        }
        return LocalizationUtils.getSettingLabel(this, setting.getSettings().getSetting(message));
    }

    public void pageBeginRender(PageEvent event_) {
    }

}
