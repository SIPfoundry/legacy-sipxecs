/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.phone.polycom;

import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.device.DeviceDefaults;
import org.sipfoundry.sipxconfig.device.DeviceVersion;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.setting.SettingEntry;

public class PolycomLineDefaults {

    private DeviceDefaults m_defaults;
    private Line m_line;

    PolycomLineDefaults(DeviceDefaults defaults, Line line) {
        m_defaults = defaults;
        m_line = line;
    }

    @SettingEntry(paths = { "reg/server/1/transport", "reg/server/2/transport" })
    public String transport() {
        DeviceVersion ver = m_line.getPhone().getDeviceVersion();
        if (ver == PolycomModel.VER_1_6) {
            return "UDPonly";
        }    
        return "DNSnaptr";
    }
    
    @SettingEntry(path = "msg.mwi/subscribe")
    public String getMwiSubscribe() {
        String uri = null;
        User u = m_line.getUser();
        if (u != null) {
            uri = u.getUserName() + '@' + m_defaults.getDomainName();
        }

        return uri;
    }

    @SettingEntry(path = "msg.mwi/callBack")
    public String getCallBack() {
        String uri = null;
        User u = m_line.getUser();
        if (u != null) {
            uri = m_defaults.getVoiceMail() + '@' + m_defaults.getDomainName();
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

    @SettingEntry(path = PolycomPhone.AUTHORIZATION_ID_PATH)
    public String getAuthorizationId() {
        return getAddress();
    }

    @SettingEntry(path = PolycomPhone.USER_ID_PATH)
    public String getAddress() {
        User u = m_line.getUser();
        if (u != null) {
            return u.getUserName();
        }
        return null;
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

    @SettingEntry(path = PolycomPhone.REGISTRATION_PATH)
    public String getRegistrationServer() {
        User u = m_line.getUser();
        if (u != null) {
            return m_line.getPhoneContext().getPhoneDefaults().getDomainName();
        }
        return null;
    }
}
