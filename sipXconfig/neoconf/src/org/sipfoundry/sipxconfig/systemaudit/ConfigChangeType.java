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

package org.sipfoundry.sipxconfig.systemaudit;

import java.util.ArrayList;
import java.util.List;

import org.apache.commons.lang.enums.Enum;

public final class ConfigChangeType extends Enum {

    public static final ConfigChangeType ALL = new ConfigChangeType("All");

    public static final ConfigChangeType LOGIN_LOGOUT = new ConfigChangeType("LoginLogout");
    public static final ConfigChangeType FEATURE = new ConfigChangeType("Feature");
    public static final ConfigChangeType SETTINGS = new ConfigChangeType("Settings");
    public static final ConfigChangeType SERVER = new ConfigChangeType("Server");
    public static final ConfigChangeType USER = new ConfigChangeType("User");
    public static final ConfigChangeType PHONE = new ConfigChangeType("Phone");
    public static final ConfigChangeType LICENSE_UPLOAD = new ConfigChangeType("LicenseUpload");

    private String m_value;

    private ConfigChangeType(String value) {
        super(value);
        this.m_value = value;
    }

    public String getValue() {
        return m_value;
    }

    /**
     * returns True if the value in the Details column needs translation
     */
    public static boolean hasTranslatedDetails(String configChangeType) {
        if (configChangeType.equals(ConfigChangeType.LOGIN_LOGOUT.getName())
                || configChangeType.equals(ConfigChangeType.SETTINGS.getName())
                || configChangeType.equals(ConfigChangeType.FEATURE.getName())) {
            return true;
        }
        return false;
    }

    /**
     * Decides whether the given action is enabled or disabled for the current
     * type
     *
     * @param configChangeAction
     * @return
     */
    public static boolean isActionDisabled(String configChangeType, ConfigChangeAction configChangeAction) {
        if (configChangeType.equals(ConfigChangeType.ALL.getName())) {
            return false;
        }
        List<ConfigChangeAction> supportedActions = new ArrayList<ConfigChangeAction>();
        // All is supported by every type
        supportedActions.add(ConfigChangeAction.ALL);
        if (configChangeType.equals(LOGIN_LOGOUT.getName())) {
            supportedActions.add(ConfigChangeAction.LOGIN);
            supportedActions.add(ConfigChangeAction.LOGOUT);
        } else if (configChangeType.equals(FEATURE.getName())) {
            supportedActions.add(ConfigChangeAction.ENABLED);
            supportedActions.add(ConfigChangeAction.DISABLED);
        } else if (configChangeType.equals(SERVER.getName())) {
            supportedActions.add(ConfigChangeAction.SEND_PROFILE);
            supportedActions.add(ConfigChangeAction.RESET_KEYS);
            supportedActions.add(ConfigChangeAction.SERVICE_RESTART);
        } else {
            supportedActions.add(ConfigChangeAction.ADDED);
            supportedActions.add(ConfigChangeAction.MODIFIED);
            supportedActions.add(ConfigChangeAction.DELETED);
        }
        return !supportedActions.contains(configChangeAction);
    }

    public static List<String> getValues() {
        List<ConfigChangeType> enumList = ConfigChangeType
                .getEnumList(ConfigChangeType.class);
        List<String> values = new ArrayList<String>();
        for (ConfigChangeType configChangeType : enumList) {
            values.add(configChangeType.getName());
        }
        return values;
    }

}
