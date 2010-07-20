/*
 *
 *
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.setting.type;
import static java.lang.String.format;

/**
 * SettingType to represent a sequence of zero or more user names separated by
 * spaces, for example: "joe 153 *81".
 * Usage might be for entering aliases.
 */
public class UsernameSequenceSetting extends StringSetting {
    //see validUsernamePatternSequence validator in tapestry.xml
    private static final String SIP_USER_ESCAPED = "(&#37;[0-9a-fA-F]{2})";
    private static final String SIP_USER_UNRESERVED = "-_.!~*'\\(\\)&amp;=+$,;?/";
    private static final String SIP_USER_CHARS = format("%s;a-zA-Z0-9", SIP_USER_UNRESERVED);
    private static final String VALID_USERNAME_PATTERN_SEQUENCE =
                             format("((([%s]|%s;)+)\\s*)*", SIP_USER_CHARS, SIP_USER_ESCAPED);
    @Override
    public String getPattern() {
        // The pattern should be like this...
        // ((([-_.!~*'\\(\\)&amp;=+$,;?/;a-zA-Z0-9]|(&#37;[0-9a-fA-F]{2});)+)\\s*)*
        return VALID_USERNAME_PATTERN_SEQUENCE;
    }

}
