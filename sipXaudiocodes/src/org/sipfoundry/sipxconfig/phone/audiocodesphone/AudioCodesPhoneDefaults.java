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

import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.device.DeviceDefaults;
import org.sipfoundry.sipxconfig.phonelog.PhoneLog;
import org.sipfoundry.sipxconfig.setting.SettingEntry;

public class AudioCodesPhoneDefaults {
    public static final String BEAN_ID = "audiocodesphone";
    public static final String USER_ID_SETTING = "voip/id";
    public static final String AUTH_ID_SETTING = "voip/auth_name";
    public static final String PASSWORD_SETTING = "voip/auth_password";
    public static final String DISPLAY_NAME = "voip/description";
    public static final String REGISTRAR_ADDRESS = "sip/sip_registrar/addr";
    public static final String OUTBOUND_PROXY_ADDRESS = "sip/sip_outbound_proxy/addr";
    public static final String PROXY_ADDRESS = "sip/proxy_address";
    public static final String MWI_SUBSCRIBE_ADDRESS = "services/msg_waiting_ind/subscribe_address";
    public static final String VOICEMAIL_ACCESS_NUMBER = "services/msg_waiting_ind/voice_mail_number";
    public static final String SYSLOG_SERVER = "system/syslog/server_address";
    public static final String TIME_SERVER_NAME = "system/ntp/primary_server_address";
    public static final String ALTERNATE_TIME_SERVER_NAME = "system/ntp/secondary_server_address";

    public static final String MIME_TYPE_PLAIN = "text/plain";

    public static final String GMT_LONDON = "+00:00 GMT London";

    private DeviceDefaults m_defaults;

    AudioCodesPhoneDefaults(DeviceDefaults defaults) {
        m_defaults = defaults;
    }

    @SettingEntry(path = TIME_SERVER_NAME)
    public String getNtpServer() {
        return m_defaults.getNtpServer();
    }

    @SettingEntry(path = ALTERNATE_TIME_SERVER_NAME)
    public String getAlternateNtpServer() {
        return m_defaults.getAlternateNtpServer();
    }

    @SettingEntry(path = SYSLOG_SERVER)
    public String getSyslogServer() {
        Address syslog = m_defaults.getAddressManager().getSingleAddress(PhoneLog.PHONELOG);
        return syslog != null ? syslog.getAddress() : null;
    }

    @SettingEntry(path = PROXY_ADDRESS)
    public String getProxyServer() {
        return m_defaults.getDomainName();
    }

    @SettingEntry(path = REGISTRAR_ADDRESS)
    public String getRegistrationServer() {
        return m_defaults.getDomainName();
    }

    @SettingEntry(path = OUTBOUND_PROXY_ADDRESS)
    public String getOutboundProxy() {
        return m_defaults.getDomainName();
    }

    @SettingEntry(path = MWI_SUBSCRIBE_ADDRESS)
    public String getMwiSubscribe() {
        return m_defaults.getDomainName();
    }

    @SettingEntry(path = VOICEMAIL_ACCESS_NUMBER)
    public String getVoiceMailNumber() {
        return m_defaults.getVoiceMail();
    }
}
