/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.setting;

public class ConditionalSettingImpl extends SettingImpl implements ConditionalSetting {
    private String m_if;
    private String m_unless;
    public String getIf() {
        return m_if;
    }
    public void setIf(String if1) {
        m_if = if1;
    }
    public String getUnless() {
        return m_unless;
    }
    public void setUnless(String unless) {
        m_unless = unless;
    }
}
