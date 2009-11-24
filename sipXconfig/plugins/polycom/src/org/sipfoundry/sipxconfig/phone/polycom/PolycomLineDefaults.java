/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.phone.polycom;

import org.sipfoundry.sipxconfig.common.SipUri;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.device.DeviceDefaults;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.setting.SettingEntry;

public class PolycomLineDefaults {

    private static final String TYPE_PRIVATE = "private";
    private static final String TYPE_SHARED = "shared";
    private final DeviceDefaults m_defaults;
    private final Line m_line;

    PolycomLineDefaults(DeviceDefaults defaults, Line line) {
        m_defaults = defaults;
        m_line = line;
    }

    @SettingEntry(path = "msg.mwi/subscribe")
    public String getMwiSubscribe() {
        String uri = null;
        User u = m_line.getUser();
        if (u != null) {
            uri = u.getUserName();
        }

        return uri;
    }

    @SettingEntry(path = "msg.mwi/callBack")
    public String getCallBack() {
        String uri = null;
        User u = m_line.getUser();
        if (u != null) {
            uri = m_defaults.getVoiceMail();
        }

        return uri;
    }

    @SettingEntry(path = "msg.mwi/callBackMode")
    public String getCallBackMode() {
        String mode = "disabled";
        User u = m_line.getUser();
        if (u != null) {
            mode = PolycomPhone.CONTACT_MODE;
        }

        return mode;
    }

    @SettingEntry(path = "reg/musicOnHold.uri")
    public String getMusicOnHoldUri() {
        String mohUri;
        User u = m_line.getUser();
        if (u != null) {
            mohUri = u.getMusicOnHoldUri();
        } else {
            mohUri = m_defaults.getMusicOnHoldUri();
        }
        return  SipUri.stripSipPrefix(mohUri);
    }

    @SettingEntry(path = PolycomPhone.AUTHORIZATION_ID_PATH)
    public String getAuthorizationId() {
        return m_line.getAuthenticationUserName();
    }

    @SettingEntry(path = PolycomPhone.USER_ID_PATH)
    public String getAddress() {
        return m_line.getUserName();
    }

    @SettingEntry(path = PolycomPhone.PASSWORD_PATH)
    public String getAuthorizationPassword() {
        User u = m_line.getUser();
        if (u != null) {
            return u.getSipPassword();
        }
        return null;
    }

    @SettingEntry(path = PolycomPhone.DISPLAY_NAME_PATH)
    public String getDisplayName() {
        User u = m_line.getUser();
        if (u != null) {
            return u.getDisplayName();
        }
        return null;
    }

    @SettingEntry(path = PolycomPhone.TYPE_PATH)
    public String getType() {
        User u = m_line.getUser();
        if (u != null) {
            return u.getIsShared() ? TYPE_SHARED : TYPE_PRIVATE;
        }
        return null;
    }

    @SettingEntry(path = PolycomPhone.THIRD_PARTY_NAME_PATH)
    public String getThirdPartyName() {
        if (TYPE_SHARED.equals(getType())) {
            return getAddress();
        }
        return null;
    }

    @SettingEntry(path = PolycomPhone.REGISTRATION_PATH)
    public String getRegistrationServer() {
        User u = m_line.getUser();
        if (u != null) {
            return m_line.getPhoneContext().getPhoneDefaults().getDomainName();
        }
        return null;
    }

    @SettingEntry(path = "line-dialplan/digitmap/routing.1/emergency.1.value")
    public String getEmergencyNumber() {
        return m_defaults.getEmergencyNumber();
    }

    @SettingEntry(path = "line-dialplan/digitmap/routing.1/address")
    public String getEmergencyRoute() {
        return m_defaults.getEmergencyAddress();
    }

    @SettingEntry(path = "line-dialplan/digitmap/routing.1/port")
    public Integer getEmergencyPort() {
        return m_defaults.getEmergencyPort();
    }

    @SettingEntry(path = "reg/outboundProxy.address")
    public String getProxyServer() {
        return m_defaults.getDomainName();
    }

    @SettingEntry(path = "reg/outboundProxy.port")
    public String getProxyPort() {
        return m_defaults.getProxyServerSipPort();
    }
}
