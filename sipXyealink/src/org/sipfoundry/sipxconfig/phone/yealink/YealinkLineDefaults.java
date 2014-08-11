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

import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.device.DeviceDefaults;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.setting.SettingEntry;

public class YealinkLineDefaults {
    private final DeviceDefaults m_defaults;
    private final Line m_line;

    YealinkLineDefaults(DeviceDefaults defaults, Line line) {
        m_defaults = defaults;
        m_line = line;
    }

    @SettingEntry(paths = {
            YealinkConstants.AUTH_ID_V6X_SETTING,
            YealinkConstants.USER_ID_V6X_SETTING,
            YealinkConstants.AUTH_ID_V7X_SETTING,
            YealinkConstants.USER_ID_V7X_SETTING
            })
    public String getUserName() {
        String userName = null;
        User user = m_line.getUser();
        if (user != null) {
            userName = user.getUserName();
        }
        return userName;
    }

    @SettingEntry(paths = {
            YealinkConstants.DISPLAY_NAME_V6X_SETTING,
            YealinkConstants.DISPLAY_NAME_V7X_SETTING
            })
    public String getDisplayName() {
        String displayName = null;
        User user = m_line.getUser();
        if (user != null) {
            displayName = user.getDisplayName();
        }
        return displayName;
    }

    @SettingEntry(paths = {
            YealinkConstants.PASSWORD_V6X_SETTING,
            YealinkConstants.PASSWORD_V7X_SETTING
            })
    public String getPassword() {
        String password = null;
        User user = m_line.getUser();
        if (user != null) {
            password = user.getSipPassword();
        }
        return password;
    }

    @SettingEntry(paths = {
            YealinkConstants.REGISTRATION_SERVER_HOST_V6X_SETTING,
            YealinkConstants.REGISTRATION_SERVER_HOST_V7X_SETTING
            })
    public String getRegistrationServer() {
        return m_defaults.getDomainName();
    }
/*@
    Returns SIP port.
    Port 5060 is internal SIP port in sipXecs by default
*/
    @SettingEntry(paths = {
            YealinkConstants.REGISTRATION_SERVER_PORT_V6X_SETTING,
            YealinkConstants.OUTBOUND_PORT_V6X_SETTING,
            YealinkConstants.BACKUP_OUTBOUND_PORT_V6X_SETTING,
            YealinkConstants.REGISTRATION_SERVER_PORT_V7X_SETTING,
            YealinkConstants.OUTBOUND_PORT_V7X_SETTING,
            YealinkConstants.BACKUP_OUTBOUND_PORT_V7X_SETTING
            })
    public Integer getRegistrationServerPort() {
        return 5060;
    }

    @SettingEntry(paths = {
            YealinkConstants.OUTBOUND_HOST_V6X_SETTING,
            YealinkConstants.BACKUP_OUTBOUND_HOST_V6X_SETTING,
            YealinkConstants.OUTBOUND_HOST_V7X_SETTING,
            YealinkConstants.BACKUP_OUTBOUND_HOST_V7X_SETTING
            })
    public String getOutboundHost() {
        Address outboundProxyAdress = m_defaults.getProxyAddress();
        if (null == outboundProxyAdress) {
            return "";
        }
        return outboundProxyAdress.getAddress();
    }

    @SettingEntry(paths = {
            YealinkConstants.VOICE_MAIL_NUMBER_V6X_SETTING,
            YealinkConstants.VOICE_MAIL_NUMBER_V7X_SETTING
            })
    public String getVoiceMail() {
        String voicemail = null;
        User u = m_line.getUser();
        if (u != null) {
            voicemail = m_defaults.getVoiceMail();
        }
        return voicemail;
    }

    @SettingEntry(paths = {
            YealinkConstants.ADVANCED_MUSIC_SERVER_URI_V6X_SETTING,
            YealinkConstants.ADVANCED_MUSIC_SERVER_URI_V7X_SETTING })
    public String getMusicServerUri() {
        String mohUri;
        User u = m_line.getUser();
        if (u != null) {
            mohUri = u.getMusicOnHoldUri();
        } else {
            mohUri = m_defaults.getMusicOnHoldUri();
        }
        return mohUri;
    }

}
