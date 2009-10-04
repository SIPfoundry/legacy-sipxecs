/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */

package org.sipfoundry.sipxconfig.im;

import org.sipfoundry.sipxconfig.common.User;

import static org.apache.commons.lang.StringUtils.defaultString;
import static org.apache.commons.lang.StringUtils.isNotBlank;

/**
 * Flyweight facade for User class that simplifies using it in the context of instant messaging
 * services
 */
public class ImAccount {

    private final User m_user;

    public ImAccount(User user) {
        m_user = user;
    }

    public boolean isEnabled() {
        //If permission manager is null there is no setting available
        //This additional filter should be done to avoid null pointer exception when no setting is available
        if (m_user.getSettings() == null) {
            return false;
        }
        return (Boolean) m_user.getSettingTypedValue("im/im-account");
    }

    /**
     * Attempts to find the best identifier that can be used as IM id.
     *
     * If configured IM id has the highest priority. If not username or an alias is used in the
     * following order: (1) non-numeric username, (2) non-numeric alias, (3) username.
     */
    public String getImId() {
        String imId = m_user.getImId();
        if (isNotBlank(imId)) {
            return imId;
        }
        String userName = m_user.getUserName();
        if (!User.isNumeric(userName)) {
            return userName;
        }
        for (String alias : m_user.getAliases()) {
            if (!User.isNumeric(alias)) {
                return alias;
            }
        }
        return userName;
    }

    public String getImDisplayName() {
        String displayName = m_user.getImDisplayName();
        if (isNotBlank(displayName)) {
            return displayName;
        }
        return defaultString(m_user.getDisplayName(), m_user.getUserName());
    }

    public String getImPassword() {
        // temporary - until we add password support
        return getImId();
    }

    public String getOnThePhoneMessage() {
        return defaultString((String) m_user.getSettingTypedValue("im/on-the-phone-message"));
    }

    public boolean advertiseSipPresence() {
        return (Boolean) m_user.getSettingTypedValue("im/advertise-sip-presence");
    }

    public boolean includeCallInfo() {
        return (Boolean) m_user.getSettingTypedValue("im/include-call-info");
    }
}
