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
import org.sipfoundry.sipxconfig.common.EnumUserType;

public final class ConfigChangeType extends Enum {

    public static final ConfigChangeType ALL = new ConfigChangeType("All", "0");

    public static final ConfigChangeType LOGIN_LOGOUT = new ConfigChangeType("LoginLogout", "1");
    public static final ConfigChangeType USER = new ConfigChangeType("User", "2");
    public static final ConfigChangeType CALL_FORWARDING = new ConfigChangeType("CallForwarding", "3");
    public static final ConfigChangeType SPEED_DIAL = new ConfigChangeType("SpeedDial", "4");
    public static final ConfigChangeType USER_SCHEDULE = new ConfigChangeType("UserSchedule", "5");
    public static final ConfigChangeType PHONE = new ConfigChangeType("Phone", "6");
    public static final ConfigChangeType LINE = new ConfigChangeType("Line", "7");
    public static final ConfigChangeType GATEWAY = new ConfigChangeType("Gateway", "8");
    public static final ConfigChangeType SBC_DEVICE = new ConfigChangeType("SbcDevice", "9");
    public static final ConfigChangeType FEATURE = new ConfigChangeType("Feature", "10");
    public static final ConfigChangeType GROUP = new ConfigChangeType("Group", "11");
    public static final ConfigChangeType CONFERENCE = new ConfigChangeType("Conference", "12");
    public static final ConfigChangeType BRANCH = new ConfigChangeType("Branch", "13");
    public static final ConfigChangeType DOMAIN = new ConfigChangeType("Domain", "14");
    public static final ConfigChangeType SETTINGS = new ConfigChangeType("Settings", "15");
    public static final ConfigChangeType AUTO_ATTENDANT = new ConfigChangeType("AutoAttendant", "16");
    public static final ConfigChangeType HUNT_GROUP = new ConfigChangeType("HuntGroup", "17");
    public static final ConfigChangeType PHONE_BOOK = new ConfigChangeType("PhoneBook", "18");
    public static final ConfigChangeType TLS_PEER = new ConfigChangeType("TLSPeer", "19");
    public static final ConfigChangeType DIALING_RULE = new ConfigChangeType("DialingRule", "20");
    public static final ConfigChangeType CALL_PARK = new ConfigChangeType("CallPark", "21");
    public static final ConfigChangeType SERVER = new ConfigChangeType("Server", "22");

    private String m_type;
    private String m_value;

    private ConfigChangeType(String type, String value) {
        super(value);
        this.m_type = type;
        this.m_value = value;
    }

    public String getValue() {
        return m_value;
    }

    public String getType() {
        return m_type;
    }

    @Override
    public String toString() {
        return m_type;
    }

    /**
     * returns True if the value in the Details column needs translation
     */
    public boolean hasTranslatedDetails() {
        if (this == ConfigChangeType.LOGIN_LOGOUT || this == ConfigChangeType.SETTINGS
                || this == ConfigChangeType.FEATURE || this == ConfigChangeType.AUTO_ATTENDANT) {
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
    public boolean isActionDisabled(ConfigChangeAction configChangeAction) {
        if (this == ConfigChangeType.ALL) {
            return false;
        }
        List<ConfigChangeAction> supportedActions = new ArrayList<ConfigChangeAction>();
        // All is supported by every type
        supportedActions.add(ConfigChangeAction.ALL);
        if (this == LOGIN_LOGOUT) {
            supportedActions.add(ConfigChangeAction.LOGIN);
            supportedActions.add(ConfigChangeAction.LOGOUT);
        } else if (this == FEATURE) {
            supportedActions.add(ConfigChangeAction.ENABLED);
            supportedActions.add(ConfigChangeAction.DISABLED);
        } else {
            supportedActions.add(ConfigChangeAction.ADDED);
            supportedActions.add(ConfigChangeAction.MODIFIED);
            supportedActions.add(ConfigChangeAction.DELETED);
        }
        return !supportedActions.contains(configChangeAction);
    }

    /**
     * Used for Hibernate type translation
     */
    public static class UserType extends EnumUserType {
        public UserType() {
            super(ConfigChangeType.class);
        }
    }

    public static ConfigChangeType getEnum(String configChangeTypeValue) {
        return (ConfigChangeType) getEnum(ConfigChangeType.class, configChangeTypeValue);
    }

}
