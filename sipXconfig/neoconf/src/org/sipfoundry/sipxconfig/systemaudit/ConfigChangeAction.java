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

import org.apache.commons.lang.enums.Enum;
import org.sipfoundry.sipxconfig.common.EnumUserType;

public final class ConfigChangeAction extends Enum {

    public static final ConfigChangeAction ALL = new ConfigChangeAction("All", "0");

    public static final ConfigChangeAction LOGIN = new ConfigChangeAction("Login", "1");
    public static final ConfigChangeAction LOGOUT = new ConfigChangeAction("Logout", "2");
    public static final ConfigChangeAction ADDED = new ConfigChangeAction("Added", "3");
    public static final ConfigChangeAction MODIFIED = new ConfigChangeAction("Modified", "4");
    public static final ConfigChangeAction DELETED = new ConfigChangeAction("Deleted", "5");
    public static final ConfigChangeAction ENABLED = new ConfigChangeAction("Enabled", "6");
    public static final ConfigChangeAction DISABLED = new ConfigChangeAction("Disabled", "7");
    public static final ConfigChangeAction SEND_PROFILE = new ConfigChangeAction("SendProfile", "8");
    public static final ConfigChangeAction RESET_KEYS = new ConfigChangeAction("ResetKeys", "9");
    public static final ConfigChangeAction SERVICE_RESTART = new ConfigChangeAction("ServiceRestart", "10");

    private String m_action;
    private String m_value;

    private ConfigChangeAction(String name, String value) {
        super(value);
        this.m_action = name;
        this.m_value = value;
    }

    public String getValue() {
        return m_value;
    }

    public String getAction() {
        return m_action;
    }

    @Override
    public String toString() {
        return m_action;
    }

    /**
     * Used for Hibernate type translation
     */
    public static class UserType extends EnumUserType {
        public UserType() {
            super(ConfigChangeAction.class);
        }
    }

    public static ConfigChangeAction getEnum(String value) {
        return (ConfigChangeAction) getEnum(ConfigChangeAction.class, value);
    }

}
