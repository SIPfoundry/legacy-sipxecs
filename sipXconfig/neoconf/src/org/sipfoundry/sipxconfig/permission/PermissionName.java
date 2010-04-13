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

import org.sipfoundry.sipxconfig.admin.dialplan.CallTag;
import org.sipfoundry.sipxconfig.permission.Permission.Type;
import org.sipfoundry.sipxconfig.setting.Group;

public enum PermissionName {
    /** application */
    SUPERADMIN(Type.APPLICATION, "superadmin"),

    TUI_CHANGE_PIN(Type.APPLICATION, "tui-change-pin"),

    PERSONAL_AUTO_ATTENDANT(Type.APPLICATION, "personal-auto-attendant"),

    MUSIC_ON_HOLD(Type.APPLICATION, "music-on-hold"),

    /** call handling */
    NINEHUNDERED_DIALING(Type.CALL, "900Dialing", CallTag.REST),

    AUTO_ATTENDANT_DIALING(Type.CALL, "AutoAttendant", CallTag.AA),

    INTERNATIONAL_DIALING(Type.CALL, "InternationalDialing", CallTag.INTN),

    LOCAL_DIALING(Type.CALL, "LocalDialing", CallTag.LOCL),

    LONG_DISTANCE_DIALING(Type.CALL, "LongDistanceDialing", CallTag.LD),

    MOBILE(Type.CALL, "Mobile", CallTag.MOB),

    RECORD_SYSTEM_PROMPTS(Type.CALL, "RecordSystemPrompts"),

    TOLL_FREE_DIALING(Type.CALL, "TollFree", CallTag.TF),

    VOICEMAIL(Type.CALL, "Voicemail", CallTag.VM),

    NO_ACCESS(Type.CALL, "NoAccess"),

    /** Voicemail */
    EXCHANGE_VOICEMAIL(Type.VOICEMAIL_SERVER, "ExchangeUMVoicemailServer"),

    FREESWITH_VOICEMAIL(Type.VOICEMAIL_SERVER, "FreeswitchVoicemailServer");

    private Type m_type;

    private String m_name;

    private CallTag m_callTag;

    PermissionName(Type type, String name) {
        this(type, name, CallTag.UNK);
    }

    PermissionName(Type type, String name, CallTag callTag) {
        m_type = type;
        m_name = name;
        m_callTag = callTag;
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

    public CallTag getCalltag() {
        return m_callTag;
    }

    public void setEnabled(Group g, boolean enable) {
        String path = getPath();
        String value = enable ? Permission.ENABLE : Permission.DISABLE;
        g.setSettingValue(path, value);
    }

    public static PermissionName findByName(String name) {
        for (PermissionName pm : PermissionName.values()) {
            if (pm.getName().equals(name)) {
                return pm;
            }
        }
        return null;
    }
}
