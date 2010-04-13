/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.components;

import java.text.SimpleDateFormat;
import java.util.Locale;
import java.util.TimeZone;

import org.apache.tapestry.form.translator.DateTranslator;

/**
 * Default date tranlsator doesn't support setting timezones
 */
public class DateWithTimezoneTranslator extends DateTranslator {

    private TimeZone m_timeZone = TimeZone.getDefault();

    public TimeZone getTimeZone() {
        return m_timeZone;
    }

    public void setTimeZone(TimeZone zone) {
        m_timeZone = zone;
    }

    public SimpleDateFormat getDateFormat(Locale locale) {
        SimpleDateFormat format = super.getDateFormat(locale);
        format.setTimeZone(getTimeZone());
        return format;
    }
}
