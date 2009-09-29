/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin;

import java.text.DateFormat;
import java.text.FieldPosition;
import java.text.Format;
import java.text.ParsePosition;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;
import java.util.Locale;
import java.util.TimeZone;

import org.apache.commons.lang.time.FastDateFormat;

/**
 * Time zone independent value of time of day (hh:mm)
 */
public class TimeOfDay {
    private int m_hrs = -1;
    private int m_min = -1;

    public TimeOfDay() {
        // default bean constructor
    }

    /**
     * Creates a TimeOfDay using the time information from a Date object
     * in the system's default time zone.
     *
     * @param date The Date to take time information from.
     */
    public TimeOfDay(Date date) {
        this(date, TimeZone.getDefault());
    }

    public TimeOfDay(Date date, TimeZone timeZone) {
        Calendar cal = Calendar.getInstance(timeZone);
        cal.setTime(date);
        m_hrs = cal.get(Calendar.HOUR_OF_DAY);
        m_min = cal.get(Calendar.MINUTE);
    }

    public TimeOfDay(int hrs, int min) {
        m_hrs = hrs;
        m_min = min;
    }

    public void setHrs(int hrs) {
        m_hrs = hrs;
    }

    public int getHrs() {
        return m_hrs;
    }

    public void setMin(int min) {
        m_min = min;
    }

    public int getMin() {
        return m_min;
    }

    public static class TimeOfDayFormat extends Format {
        private Locale m_locale;

        public TimeOfDayFormat() {
            // use default locale
        }

        public TimeOfDayFormat(Locale locale) {
            m_locale = locale;
        }

        public StringBuffer format(Object obj, StringBuffer toAppendTo, FieldPosition pos) {
            if (!(obj instanceof TimeOfDay)) {
                throw new IllegalArgumentException("Unknown class: "
                        + (obj == null ? "<null>" : obj.getClass().getName()));
            }
            TimeOfDay tod = (TimeOfDay) obj;
            FastDateFormat format = FastDateFormat
                    .getTimeInstance(FastDateFormat.SHORT, m_locale);
            Calendar date = Calendar.getInstance();
            date.set(Calendar.HOUR_OF_DAY, tod.getHrs());
            date.set(Calendar.MINUTE, tod.getMin());
            return format.format(date, toAppendTo, pos);
        }

        public Object parseObject(String source, ParsePosition pos) {
            DateFormat format = DateFormat.getTimeInstance(DateFormat.SHORT, m_locale);
            Date date = (Date) format.parseObject(source, pos);
            if (date == null) {
                // try simple format
                SimpleDateFormat simpleFormat = new SimpleDateFormat("HH:mm", m_locale);
                date = (Date) simpleFormat.parseObject(source, pos);
                if (date == null) {
                    return null;
                }
            }
            Calendar calendar = Calendar.getInstance();
            calendar.setTime(date);
            TimeOfDay tod = new TimeOfDay();
            tod.setHrs(calendar.get(Calendar.HOUR_OF_DAY));
            tod.setMin(calendar.get(Calendar.MINUTE));
            return tod;
        }

        public String toLocalizedPattern() {
            SimpleDateFormat format = new SimpleDateFormat("hh:mm a", m_locale);
            return format.toLocalizedPattern().toUpperCase();
        }
    }
}
