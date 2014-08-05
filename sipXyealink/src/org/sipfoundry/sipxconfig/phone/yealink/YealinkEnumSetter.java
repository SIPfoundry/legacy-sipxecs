/*
 * Copyright (c) 2013 SibTelCom, JSC (SIPLABS Communications). All rights reserved.
 * Contributed to SIPfoundry and eZuce, Inc. under a Contributor Agreement.
 *
 * Developed by Konstantin S. Vishnivetsky
 *
 * This library or application is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Affero General Public License (AGPL) as published by the Free
 * Software Foundation; either version 3 of the License, or (at your option) any later version.
 *
 * This library or application is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License (AGPL) for
 * more details.
 *
 */

package org.sipfoundry.sipxconfig.phone.yealink;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.setting.AbstractSettingVisitor;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.type.EnumSetting;
import org.sipfoundry.sipxconfig.setting.type.MultiEnumSetting;

public abstract class YealinkEnumSetter extends AbstractSettingVisitor {
    private static final Log LOG = LogFactory.getLog(YealinkEnumSetter.class);
    private String m_pattern;

    public YealinkEnumSetter(String pattern) {
        m_pattern = pattern;
    }

    public void setPattern(String value) {
        m_pattern = value;
    }

    public String getPattern() {
        return m_pattern;
    }

    @Override
    public void visitSetting(Setting setting) {
        if ((setting.getType() instanceof EnumSetting) | (setting.getType() instanceof MultiEnumSetting)) {
            if (setting.getPath().matches(getPattern())) {
                if (setting.getType().getName().equals("enum")) {
                    addEnums(setting.getName(), setting.getIndex(), (EnumSetting) setting.getType());
                } else if (setting.getType().getName().equals("multiEnum")) {
                    addMultiEnums(setting.getName(), setting.getIndex(), (MultiEnumSetting) setting.getType());
                }
            }
        }
    }

    // Override this method in implementation to fill enum
    protected void addEnums(String settingName, Integer settingIndex, EnumSetting setting) {
    }

    // Override this method in implementation to fill multi-enum
    protected void addMultiEnums(String settingName, Integer settingIndex, MultiEnumSetting setting) {
    }

};
