/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.permission;

import org.sipfoundry.sipxconfig.permission.Permission.Type;
import org.sipfoundry.sipxconfig.setting.Group;

public enum PermissionName {
    /** application */
    SUPERADMIN(Type.APPLICATION, "superadmin"),

    TUI_CHANGE_PIN(Type.APPLICATION, "tui-change-pin"),

    PERSONAL_AUTO_ATTENDANT(Type.APPLICATION, "personal-auto-attendant"),

    /** call handling */
    NINEHUNDERED_DIALING(Type.CALL, "900Dialing"),

    AUTO_ATTENDANT_DIALING(Type.CALL, "AutoAttendant"),

    INTERNATIONAL_DIALING(Type.CALL, "InternationalDialing"),

    LOCAL_DIALING(Type.CALL, "LocalDialing"),

    LONG_DISTANCE_DIALING(Type.CALL, "LongDistanceDialing"),

    MOBILE(Type.CALL, "Mobile"),

    RECORD_SYSTEM_PROMPTS(Type.CALL, "RecordSystemPrompts"),

    TOLL_FREE_DIALING(Type.CALL, "TollFree"),

    VOICEMAIL(Type.CALL, "Voicemail"),

    NO_ACCESS(Type.CALL, "NoAccess"),

    /** Voicemail */
    SIPX_VOICEMAIL(Type.VOICEMAIL_SERVER, "SipXVoicemailServer"),

    EXCHANGE_VOICEMAIL(Type.VOICEMAIL_SERVER, "ExchangeUMVoicemailServer");

    private Type m_type;

    private String m_name;

    PermissionName(Type type, String name) {
        m_type = type;
        m_name = name;
    }

    /**
     * Call to retrieve permission from setting model
     */
    public String getPath() {
        return Permission.getSettingsPath(m_type, m_name);
    }

    /**
     * Call to use permisison in mapping/fallback/auth rules
     */
    public String getName() {
        return m_name;
    }

    public void setEnabled(Group g, boolean enable) {
        String path = getPath();
        String value = enable ? Permission.ENABLE : Permission.DISABLE;
        g.setSettingValue(path, value);
    }
}
