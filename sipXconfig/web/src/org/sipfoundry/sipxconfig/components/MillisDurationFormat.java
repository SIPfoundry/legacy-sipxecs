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

import java.text.FieldPosition;
import java.text.Format;
import java.text.MessageFormat;
import java.text.ParsePosition;
import java.util.Arrays;
import java.util.Locale;
import java.util.ResourceBundle;

import org.apache.commons.lang.ObjectUtils;
import org.apache.commons.lang.time.DateUtils;

public class MillisDurationFormat extends Format {

    public static final int DAYS = 0;
    public static final int HOURS = 1;
    public static final int MINUTES = 2;
    public static final int SECONDS = 3;

    private static final String SEPARATOR = ", ";
    private static final long[] INTERVALS = {
        DateUtils.MILLIS_PER_DAY, DateUtils.MILLIS_PER_HOUR, DateUtils.MILLIS_PER_MINUTE,
        DateUtils.MILLIS_PER_SECOND
    };

    /**
     * Used to properly compute carry over. Keeps HOURS_PER_DAY, MINUTS_PER_HOUR,
     * SECONDS_PER_MINUTE values.
     */
    private static final long[] MAX = {
        -1, 24, 60, 60
    };

    private static final String[] LABELS = {
        "format.days", "format.hours", "format.minutes", "format.seconds"
    };

    private int m_maxField = DAYS;
    private Format[] m_formats;
    private String m_separator = SEPARATOR;
    private boolean m_showZero;
    private Locale m_locale;

    public void setMaxField(int maxField) {
        m_maxField = maxField;
    }

    public void setSeparator(String separator) {
        m_separator = separator;
    }

    public void setShowZero(boolean showZero) {
        m_showZero = showZero;
    }

    public void setLocale(Locale locale) {
        if (ObjectUtils.equals(locale, m_locale)) {
            return;
        }
        initLocale(locale);
    }

    public Object parseObject(String source, ParsePosition pos) {
        throw new UnsupportedOperationException();
    }

    public StringBuffer format(Object obj, StringBuffer toAppendTo, FieldPosition pos) {
        // only works for numbers - class exception otherwise
        Number millisNumber = (Number) obj;
        long millis = millisNumber.longValue();

        if (m_formats == null) {
            initLocale(null);
        }

        if (millis < DateUtils.MILLIS_PER_SECOND && !m_showZero) {
            return toAppendTo;
        }

        boolean added = false;
        long[] result = calculate(millis);
        for (int i = 0; i < result.length; i++) {
            long l = result[i];
            if (l >= 0 || added || i == result.length - 1) {
                if (added) {
                    toAppendTo.append(m_separator);
                }
                Object params = new Object[] {
                    l
                };
                m_formats[i].format(params, toAppendTo, pos);
                added = true;
            }
        }
        return toAppendTo;
    }

    private long[] calculate(long m) {
        long millis = m;
        long[] result = new long[INTERVALS.length];
        Arrays.fill(result, -1);
        for (int i = m_maxField; i < INTERVALS.length; i++) {
            long interval = INTERVALS[i];
            long units = millis / interval;
            if (units > 0) {
                millis = millis % interval;
            }
            result[i] = units;
        }
        // add carry - 0.45s is displayed as 1 second
        if (millis > 0) {
            for (int i = INTERVALS.length - 1; i >= m_maxField; i--) {
                result[i]++;
                if (result[i] >= MAX[i] && i > m_maxField) {
                    result[i] = 0;
                } else {
                    break;
                }
            }
        }

        return result;
    }

    private void initLocale(Locale locale) {
        m_locale = locale;
        String[] labels = new String[LABELS.length];
        if (m_locale == null) {
            Arrays.fill(labels, "{0,number,#,#00}");
        } else {
            ResourceBundle bundle = ResourceBundle.getBundle(getClass().getName(), m_locale,
                    getClass().getClassLoader());
            for (int i = 0; i < LABELS.length; i++) {
                labels[i] = bundle.getString(LABELS[i]);
            }
        }
        m_formats = new MessageFormat[LABELS.length];
        for (int i = 0; i < labels.length; i++) {
            m_formats[i] = new MessageFormat(labels[i]);
        }
    }
}
