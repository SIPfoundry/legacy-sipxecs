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

import java.util.Locale;

import javax.xml.bind.annotation.XmlRootElement;
import javax.xml.bind.annotation.XmlType;

import org.sipfoundry.sipxconfig.setting.Setting;
import org.springframework.context.MessageSource;
import org.springframework.context.NoSuchMessageException;

@XmlRootElement(name = "Setting")
@XmlType(propOrder = {
        "path", "type", "value", "defaultValue", "label", "description"
        })
public class SettingBean {
    private String m_path;
    private String m_type;
    private String m_defaultValue;
    private String m_value;
    private String m_label;
    private String m_description;

    public String getPath() {
        return m_path;
    }

    public void setPath(String path) {
        m_path = path;
    }

    public String getType() {
        return m_type;
    }

    public void setType(String type) {
        m_type = type;
    }

    public String getDefaultValue() {
        return m_defaultValue;
    }

    public void setDefaultValue(String value) {
        m_defaultValue = value;
    }

    public String getValue() {
        return m_value;
    }

    public void setValue(String value) {
        m_value = value;
    }

    public String getLabel() {
        return m_label;
    }

    public void setLabel(String label) {
        m_label = label;
    }

    public String getDescription() {
        return m_description;
    }

    public void setDescription(String description) {
        m_description = description;
    }

    public static SettingBean convertSetting(Setting setting, Locale locale) {
        SettingBean bean = new SettingBean();
        bean.setPath(setting.getPath());
        bean.setType(setting.getType().getName());
        bean.setDefaultValue(setting.getDefaultValue());
        bean.setValue(setting.getValue());
        bean.setLabel(getMessage(setting.getMessageSource(), setting.getLabelKey(), locale));
        bean.setDescription(getMessage(setting.getMessageSource(), setting.getDescriptionKey(), locale));
        return bean;
    }

    private static String getMessage(MessageSource ms, String key, Locale locale) {
        try {
            return ms.getMessage(key, null, locale);
        } catch (NoSuchMessageException e) {
            return null;
        }
    }
}
