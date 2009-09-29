/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.dialplan;

import org.apache.commons.lang.ArrayUtils;
import org.apache.commons.lang.StringUtils;
import org.apache.commons.lang.enums.Enum;
import org.sipfoundry.sipxconfig.common.EnumUserType;

public class CallDigits extends Enum {
    public static final CallDigits NO_DIGITS = new CallDigits("nodigits");
    public static final CallDigits VARIABLE_DIGITS = new CallDigits("vdigits");
    public static final CallDigits FIXED_DIGITS = new CallDigits("digits");

    public static final String ESCAPED_SUFFIX = "-escaped";

    /** Characters that can start dial pattern regex */
    static final String START_REGEX_CHARS = "[.x";
    static final char ESCAPE = '\\';

    public CallDigits(String name) {
        super(name);
    }

    /**
     * Used for Hibernate type translation
     */
    public static class UserType extends EnumUserType {
        public UserType() {
            super(CallDigits.class);
        }
    }

    /**
     * VARIABLE_DIGITS means - the suffix of the dial string that starts with the first variable
     * digit matched.
     *
     * @param prefix in a dial pattern
     * @return prefix in a pattern transformed by me
     */
    public String transformPrefix(String prefix) {
        if (equals(NO_DIGITS)) {
            return StringUtils.EMPTY;
        }
        if (equals(FIXED_DIGITS)) {
            return prefix;
        }
        int varStart = findFirstNonEscapedSpecialChar(prefix, START_REGEX_CHARS, ESCAPE);
        if (varStart > 0) {
            return prefix.substring(varStart);
        }
        return StringUtils.EMPTY;
    }

    /**
     * @param digits number of digits in dial pattern
     * @return number of digits in a pattern transformed by me
     */
    public int transformDigits(int digits) {
        if (equals(NO_DIGITS)) {
            return 0;
        }
        return digits;
    }

    /**
     * When used in URL transforms for media server tell media server that parameter has been URL
     * escaped.
     */
    public String getEscapedName() {
        return getName() + ESCAPED_SUFFIX;
    }

    /**
     * Finds a first characted in the pattern string that is a special character and is not
     * escaped by escape character. Escape character is escaped by itself.
     *
     * @param pattern
     * @param special set of special characters to look for
     * @param escape
     * @return index of first non escaped special character, -1 if no such characters are in the
     *         string of if all of them are escaped
     */
    static int findFirstNonEscapedSpecialChar(String pattern, String special, char escape) {
        char[] specialChars = special.toCharArray();
        boolean inEscape = false;
        for (int i = 0; i < pattern.length(); i++) {
            char c = pattern.charAt(i);
            if (c == escape) {
                inEscape = !inEscape;
                continue;
            }
            if (!inEscape && ArrayUtils.indexOf(specialChars, c) >= 0) {
                return i;
            }
            inEscape = false;
        }
        return -1;
    }
}
