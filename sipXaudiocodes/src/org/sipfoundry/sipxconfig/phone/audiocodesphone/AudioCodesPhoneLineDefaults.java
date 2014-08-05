/*
 * Initial Version Copyright (C) 2011 AudioCodes, All Rights Reserved.
 * Licensed to the User under the LGPL license.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.phone.audiocodesphone;

import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.setting.SettingEntry;

public class AudioCodesPhoneLineDefaults {
    public static final String USER_ID_SETTING = "voip/id";
    public static final String AUTH_ID_SETTING = "voip/auth_name";
    public static final String PASSWORD_SETTING = "voip/auth_password";
    public static final String DISPLAY_NAME = "voip/description";
    public static final String MWI_SUBSCRIBE_ADDRESS = "voip/services/msg_waiting_ind/subscribe_address";
    public static final String VOICEMAIL_ACCESS_NUMBER = "voip/services/msg_waiting_ind/voice_mail_number";
    public static final String SYSLOG_SERVER = "system/syslog/server_address";
    public static final String TIME_SERVER_NAME = "systes/ntp/primary_server_address";
    public static final String ALTERNATE_TIME_SERVER_NAME = "system/ntp/secondary_server_address";

    private final Line m_line;

    AudioCodesPhoneLineDefaults(Line line) {
        m_line = line;
    }

    @SettingEntry(path = USER_ID_SETTING)
    public String getUserName() {
        String userName = null;
        User user = m_line.getUser();
        if (user != null) {
            userName = user.getUserName();
        }
        return userName;
    }

    @SettingEntry(path = AUTH_ID_SETTING)
    public String getAuthId() {
        return m_line.getAuthenticationUserName();
    }

    @SettingEntry(path = PASSWORD_SETTING)
    public String getPassword() {
        String password = null;
        User user = m_line.getUser();
        if (user != null) {
            password = user.getSipPassword();
        }
        return password;
    }

    @SettingEntry(path = DISPLAY_NAME)
    public String getDisplayName() {
        String displayName = null;
        User user = m_line.getUser();
        if (user != null) {
            displayName = user.getDisplayName();
            if (displayName != null) {
                if (displayName.length() > 22) {
                    displayName = displayName.substring(0, 22);
                }
            }
        }

        return displayName;
    }
}
