/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.setting.type;

/**
 * Specialized subclass of the String type to allow for verifying SIP numbers.
 *
 * Domain is optional.
 *
 * HACK:Similar patterns are defined in tapestry.xml but there is not way to access them easily
 * from here.
 */
public class SipUriSetting extends StringSetting {
    public static final String SIP_USER_UNRESERVED = "-_.!~*\'\\(\\)&=+$,;?/";
    public static final String SIP_USER_CHARS = "[a-zA-Z0-9" + SIP_USER_UNRESERVED + "]";
    public static final String SIP_USER_ESCAPED = "%[0-9a-fA-F]{2}";
    public static final String USER_NAME = "(" + SIP_USER_CHARS + "|" + SIP_USER_ESCAPED + ")+";

    private static final String OPTIONAL_DOMAIN = "(@(\\w[-._\\w]*\\w\\.\\w{2,6})+)?";

    private static final String FULL_SIP_URI = USER_NAME + OPTIONAL_DOMAIN;

    private boolean m_userPartOnly;

    public SipUriSetting() {
    }

    @Override
    public String getPattern() {
        return m_userPartOnly ? USER_NAME : FULL_SIP_URI;
    }

    public void setUserPartOnly(boolean userPartOnly) {
        m_userPartOnly = userPartOnly;
    }
}
