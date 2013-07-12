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

import static org.apache.commons.lang.StringUtils.defaultString;
import static org.apache.commons.lang.StringUtils.isNotBlank;

import org.sipfoundry.sipxconfig.common.AbstractUser;
import org.sipfoundry.sipxconfig.common.User;

/**
 * Flyweight facade for User class that simplifies using it in the context of instant messaging
 * services
 */
public class ImAccount {
    public static final String IM_ACCOUNT = "im/im-account";

    private final AbstractUser m_user;

    public ImAccount(AbstractUser user) {
        m_user = user;
    }

    public boolean isEnabled() {
        return (Boolean) m_user.getSettingTypedValue(IM_ACCOUNT);
    }

    public void setEnabled(boolean enabled) {
        m_user.setSettingTypedValue(IM_ACCOUNT, enabled);
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
        imId = getDefaultImId();
        setImId(imId);
        return imId;
    }

    /**
     * Update user IM ID name.
     *
     * If the new value happens to be the same as default, we are removing user set value.
     * Otherwise we update to a new value.
     */
    public void setImId(String imId) {
        m_user.setImId(imId);
    }

    public String getDefaultImId() {
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
        return getDefaultImDisplayName();
    }

    /**
     * Always save imDisplayName in mongo including the default value.
     * This is needed in openfire plugins to be available
     * for vcard conversion
     */
    public void setImDisplayName(String imDisplayName) {
        m_user.setImDisplayName(imDisplayName);
    }

    public String getDefaultImDisplayName() {
        return defaultString(m_user.getDisplayName(), m_user.getUserName());
    }

    public String getEmailAddress() {
        return m_user.getEmailAddress();
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

    public boolean isForwardOnDnd() {
        return (Boolean) m_user.getSettingTypedValue("im/fwd-vm-on-dnd");
    }
}
