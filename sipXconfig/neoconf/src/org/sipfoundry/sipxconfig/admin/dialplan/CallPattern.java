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

import org.apache.commons.lang.StringUtils;

/**
 * CallPattern
 */
public class CallPattern {
    private CallDigits m_digits;
    private String m_prefix;

    public CallPattern() {
        this(StringUtils.EMPTY, CallDigits.NO_DIGITS);
    }

    public CallPattern(String prefix, CallDigits digits) {
        m_prefix = prefix;
        m_digits = digits;
    }

    public CallDigits getDigits() {
        return m_digits;
    }

    public void setDigits(CallDigits digits) {
        m_digits = digits;
    }

    public String getPrefix() {
        return m_prefix;
    }

    public void setPrefix(String prefix) {
        m_prefix = StringUtils.defaultString(prefix);
    }

    public String calculatePattern() {
        String digits = StringUtils.EMPTY;
        if (!m_digits.equals(CallDigits.NO_DIGITS)) {
            digits = "{" + m_digits.getName() + "}";
        }
        return m_prefix + digits;
    }

    /**
     * Transforms dial pattern according to call pattern setting
     *
     * VARIABLE_DIGITS means - the suffix of the dial string that starts with the first variable
     * digit matched.
     *
     * @param from dial pattern to transform
     * @return resulting dial pattern
     */
    public DialPattern transform(DialPattern from) {
        String toPrefix = m_digits.transformPrefix(from.getPrefix());
        int toDigits = m_digits.transformDigits(from.getDigits());
        return new DialPattern(m_prefix + toPrefix, toDigits);
    }
}
