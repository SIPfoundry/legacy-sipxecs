/*
 *
 *
 * Copyright (C) 2004-2009 iscoord ltd.
 * Beustweg 12, 8032 Zurich, Switzerland
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */

package org.sipfoundry.sipxconfig.phone.isphone;

import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.device.DeviceDefaults;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.setting.SettingEntry;

import static org.sipfoundry.sipxconfig.phone.isphone.IsphoneConstants.ACCOUNT_NAME;
import static org.sipfoundry.sipxconfig.phone.isphone.IsphoneConstants.AUTHENTICATION_PASSWORD;
import static org.sipfoundry.sipxconfig.phone.isphone.IsphoneConstants.AUTHENTICATION_USERNAME;
import static org.sipfoundry.sipxconfig.phone.isphone.IsphoneConstants.DISPLAY_NAME;
import static org.sipfoundry.sipxconfig.phone.isphone.IsphoneConstants.DOMAIN;
import static org.sipfoundry.sipxconfig.phone.isphone.IsphoneConstants.PROXY;
import static org.sipfoundry.sipxconfig.phone.isphone.IsphoneConstants.SIP_ID;
import static org.sipfoundry.sipxconfig.phone.isphone.IsphoneConstants.VOICE_MAIL_ACCESS_CODE;

public class IsphoneLineDefaults {
    private Line m_line;
    private DeviceDefaults m_defaults;

    IsphoneLineDefaults(DeviceDefaults defaults, Line line) {


        m_line = line;
        m_defaults = defaults;
    }

    @SettingEntry(path = DOMAIN)
    public String getDomain() {
        String domain = null;
        User user = m_line.getUser();
        if (user != null) {
            // XPB-398 This forces TCP, this is defined in conjunction with
            // "transport=udp"
            // This is benign w/o SRV, but required w/SRV
            // XCF-1680 - IFF this still needs to be set, this should domain
            // name to allow
            // for HA systems. Remember, domain name is really FQHN for non SRV
            // systems
            domain = m_defaults.getDomainName();
        }

        return domain;
    }

    @SettingEntry(path = PROXY)
    public String getProxy() {
        String proxy = null;
        User user = m_line.getUser();
        if (user != null) {
            proxy = m_defaults.getDomainName();
        }

        return proxy;
    }

    @SettingEntry(path = AUTHENTICATION_USERNAME)
    public String getUserName() {
        String username = null;
        User user = m_line.getUser();
        if (user != null) {
            username = user.getUserName();
        }

        return username;
    }

    @SettingEntry(path = SIP_ID)
    public String getSipId() {
        String username = null;
        User user = m_line.getUser();
        if (user != null) {
            username = user.getUserName();
        }

        return username;
    }

    @SettingEntry(path = VOICE_MAIL_ACCESS_CODE)
    public String getVoiceMailAccessCode() {
        return "";
    }

    @SettingEntry(path = AUTHENTICATION_PASSWORD)
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
        }

        return displayName;
    }

    @SettingEntry(path = ACCOUNT_NAME)
    public String getName() {

        String displayName = null;
        User user = m_line.getUser();
        if (user != null) {
            displayName = user.getUserName();
        }

        return displayName;
    }

}
