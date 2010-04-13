/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.common;


public class UserCallerAliasInfo {
    public static final String ANONYMOUS_CALLER_ALIAS = "caller-alias/anonymous-caller-alias";
    public static final String EXTERNAL_NUMBER = "caller-alias/external-number";

    private boolean m_anonymous;

    private String m_externalNumber;

    public UserCallerAliasInfo(User user) {
        m_anonymous = (Boolean) user.getSettingTypedValue(ANONYMOUS_CALLER_ALIAS);
        m_externalNumber = (String) user.getSettingTypedValue(EXTERNAL_NUMBER);
    }

    public String getExternalNumber() {
        return m_externalNumber;
    }

    public boolean isAnonymous() {
        return m_anonymous;
    }
}
