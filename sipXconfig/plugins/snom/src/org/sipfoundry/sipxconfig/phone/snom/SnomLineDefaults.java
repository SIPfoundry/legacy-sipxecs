/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */

package org.sipfoundry.sipxconfig.phone.snom;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.common.SipUri;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.device.DeviceDefaults;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.setting.SettingEntry;
import org.sipfoundry.sipxconfig.speeddial.SpeedDial;

import static org.sipfoundry.sipxconfig.phone.snom.SnomConstants.AUTH_NAME;
import static org.sipfoundry.sipxconfig.phone.snom.SnomConstants.DISPLAY_NAME;
import static org.sipfoundry.sipxconfig.phone.snom.SnomConstants.EV_LIST_SUBSCR;
import static org.sipfoundry.sipxconfig.phone.snom.SnomConstants.EV_LIST_URI;
import static org.sipfoundry.sipxconfig.phone.snom.SnomConstants.MAILBOX;
import static org.sipfoundry.sipxconfig.phone.snom.SnomConstants.OUTBOUND_PROXY;
import static org.sipfoundry.sipxconfig.phone.snom.SnomConstants.PASSWORD;
import static org.sipfoundry.sipxconfig.phone.snom.SnomConstants.USER_HOST;
import static org.sipfoundry.sipxconfig.phone.snom.SnomConstants.USER_MOH;
import static org.sipfoundry.sipxconfig.phone.snom.SnomConstants.USER_NAME;

public class SnomLineDefaults {
    private final Line m_line;
    private final DeviceDefaults m_defaults;
    private final SpeedDial m_speedDial;

    SnomLineDefaults(DeviceDefaults defaults, Line line, SpeedDial speedDial) {
        m_line = line;
        m_defaults = defaults;
        m_speedDial = speedDial;
    }

    @SettingEntry(path = USER_HOST)
    public String getUserHost() {
        String registrationUri = StringUtils.EMPTY;
        User u = m_line.getUser();
        if (u != null) {
            String domainName = m_defaults.getDomainName();
            registrationUri = domainName;
        }
        return registrationUri;
    }

    @SettingEntry(path = OUTBOUND_PROXY)
    public String getOutboundProxy() {
        String outboundProxy = null;
        User user = m_line.getUser();
        if (user != null) {
            // XPB-398 This forces TCP, this is defined in conjunction with
            // "transport=udp"
            // This is benign w/o SRV, but required w/SRV
            // XCF-1680 - IFF this still needs to be set, this should domain
            // name to allow
            // for HA systems. Remember, domain name is really FQHN for non SRV
            // systems
            outboundProxy = m_defaults.getDomainName();
        }

        return outboundProxy;
    }

    @SettingEntry(path = MAILBOX)
    public String getMailbox() {
        // XCF-722 Setting this to the mailbox (e.g. 101) would fix issue
        // where mailbox button on phone calls voicemail server, but would
        // break MWI subscription because SUBSCRIBE message fails
        // authentication unless this value is user's username
        return getUserName();
    }

    @SettingEntry(paths = { USER_NAME, AUTH_NAME })
    public String getUserName() {
        String username = null;
        User user = m_line.getUser();
        if (user != null) {
            username = user.getUserName();
        }

        return username;
    }

    @SettingEntry(path = PASSWORD)
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

    @SettingEntry(path = USER_MOH)
    public String getUserMoh() {
        User u = m_line.getUser();
        if (u != null) {
            return  u.getMusicOnHoldUri();
        }

        return m_defaults.getMusicOnHoldUri();
    }

    @SettingEntry(path = EV_LIST_SUBSCR)
    public boolean getEventListSubscription() {
        if (!m_line.getPhone().getModelDefinitions().contains("blf")) {
            return false;
        }
        return isBlfDefined();
    }

    @SettingEntry(path = EV_LIST_URI)
    public String getRlsUri() {
        if (!isBlfDefined()) {
            return null;
        }
        return SipUri.format(m_speedDial.getResourceListId(false), m_defaults.getDomainName(), false);
    }

    private boolean isBlfDefined() {
        return m_speedDial != null && m_speedDial.isBlf();
    }
}
