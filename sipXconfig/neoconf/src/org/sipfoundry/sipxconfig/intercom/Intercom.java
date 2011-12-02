/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.intercom;

import org.apache.commons.lang.time.DateUtils;
import org.sipfoundry.sipxconfig.setting.BeanWithGroups;
import org.sipfoundry.sipxconfig.setting.Setting;

/**
 * The Intercom class represents an intercom (auto-answer) configuration.
 */
// BeanWithGroups is overkill here because it inherits from BeanWithSettings and
// Intercom doesn't have settings.  But there is no easy way to fix that, given
// that Java doesn't support mixins or multiple inheritance of implementations.
public class Intercom extends BeanWithGroups {
    private boolean m_enabled;
    private String m_prefix;
    private int m_timeout;
    private String m_code;

    @Override
    protected Setting loadSettings() {
        return null;
    }

    /** Return the code that identifies an auto-answer configuration to the phone */
    public String getCode() {
        return m_code;
    }

    /** Set the code that identifies an auto-answer configuration to the phone */
    public void setCode(String code) {
        m_code = code;
    }

    public boolean isEnabled() {
        return m_enabled;
    }

    public void setEnabled(boolean enabled) {
        m_enabled = enabled;
    }

    /** Return the prefix used to place intercom calls */
    public String getPrefix() {
        return m_prefix;
    }

    /** Set the prefix used to place intercom calls */
    public void setPrefix(String prefix) {
        m_prefix = prefix;
    }

    /** Return the timeout (milliseconds) after which the phone auto-answers */
    public int getTimeout() {
        return m_timeout;
    }

    /** Set the timeout (milliseconds) after which the phone auto-answers */
    public void setTimeout(int timeout) {
        m_timeout = timeout;
    }

    /** Return the timeout (seconds) after which the phone auto-answers */
    public int getTimeoutInSeconds() {
        return getTimeout() / (int) DateUtils.MILLIS_PER_SECOND;
    }

    /** Set the timeout (seconds) after which the phone auto-answers */
    public void setTimeoutInSeconds(int timeout) {
        setTimeout(timeout * (int) DateUtils.MILLIS_PER_SECOND);
    }
}
