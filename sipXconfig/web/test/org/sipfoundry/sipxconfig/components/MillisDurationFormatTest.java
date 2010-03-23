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

import java.util.Locale;

import junit.framework.TestCase;

import org.apache.commons.lang.time.DateUtils;

public class MillisDurationFormatTest extends TestCase {
    private MillisDurationFormat m_format;

    protected void setUp() throws Exception {
        m_format = new MillisDurationFormat();
    }

    public void testZeroFormat() {
        m_format.setLocale(Locale.US);
        assertEquals("", m_format.format(new Long(0)));
        assertEquals("", m_format.format(new Long(1)));
        assertEquals("", m_format.format(new Long(999)));

        m_format.setShowZero(true);

        assertEquals("0 days, 0 hours, 0 minutes, 0 seconds", m_format.format(new Long(0)));
        assertEquals("0 days, 0 hours, 0 minutes, 1 second", m_format.format(new Long(1)));
        assertEquals("0 days, 0 hours, 0 minutes, 1 second", m_format.format(new Long(999)));

        m_format.setMaxField(MillisDurationFormat.MINUTES);
        assertEquals("0 minutes, 0 seconds", m_format.format(new Long(0)));
        assertEquals("0 minutes, 1 second", m_format.format(new Long(1)));
    }

    public void testFormat() {
        m_format.setLocale(Locale.US);
        assertEquals("0 days, 0 hours, 0 minutes, 1 second", m_format.format(new Long(1000)));

        assertEquals("0 days, 0 hours, 0 minutes, 5 seconds", m_format.format(new Long(5000)));
        assertEquals("0 days, 0 hours, 0 minutes, 6 seconds", m_format.format(new Long(5999)));

        assertEquals("0 days, 0 hours, 1 minute, 5 seconds", m_format.format(new Long(60000 + 5000)));

        assertEquals("0 days, 0 hours, 2 minutes, 0 seconds", m_format.format(new Long(2 * 60000)));

        assertEquals("0 days, 0 hours, 2 minutes, 5 seconds", m_format.format(new Long(2 * 60000 + 5000)));

        assertEquals("0 days, 3 hours, 0 minutes, 6 seconds", m_format.format(new Long(
                3 * DateUtils.MILLIS_PER_HOUR + 5050)));
        assertEquals("5 days, 0 hours, 1 minute, 8 seconds", m_format.format(new Long(5
                * DateUtils.MILLIS_PER_DAY + DateUtils.MILLIS_PER_MINUTE + 7070)));
    }

    public void testFormatPolish() {
        m_format.setLocale(new Locale("pl","PL",""));
        assertEquals("0 dni, 0 godzin, 0 minut, 1 sekunda", m_format.format(new Long(1000)));

        assertEquals("0 dni, 0 godzin, 0 minut, 5 sekund", m_format.format(new Long(5000)));
        assertEquals("0 dni, 0 godzin, 0 minut, 6 sekund", m_format.format(new Long(5999)));

        assertEquals("0 dni, 0 godzin, 1 minuta, 4 sekundy", m_format.format(new Long(60000 + 4000)));
        assertEquals("0 dni, 0 godzin, 1 minuta, 5 sekund", m_format.format(new Long(60000 + 5000)));

        assertEquals("0 dni, 0 godzin, 2 minuty, 0 sekund", m_format.format(new Long(2 * 60000)));

        assertEquals("0 dni, 0 godzin, 2 minuty, 5 sekund", m_format.format(new Long(2 * 60000 + 5000)));

        assertEquals("0 dni, 3 godziny, 0 minut, 6 sekund", m_format.format(new Long(
                3 * DateUtils.MILLIS_PER_HOUR + 5050)));
        assertEquals("5 dni, 0 godzin, 1 minuta, 8 sekund", m_format.format(new Long(5
                * DateUtils.MILLIS_PER_DAY + DateUtils.MILLIS_PER_MINUTE + 7070)));

        m_format.setMaxField(MillisDurationFormat.SECONDS);
        assertEquals("1 sekunda", m_format.format(new Long(1000)));


    }


    public void testFormatSeconds() {
        m_format.setLocale(Locale.US);
        m_format.setMaxField(MillisDurationFormat.SECONDS);
        assertEquals("", m_format.format(new Long(0)));
        assertEquals("", m_format.format(new Long(1)));
        assertEquals("", m_format.format(new Long(999)));
        assertEquals("1 second", m_format.format(new Long(1000)));

        assertEquals("5 seconds", m_format.format(new Long(5000)));
        assertEquals("6 seconds", m_format.format(new Long(5999)));

        assertEquals("65 seconds", m_format.format(new Long(60000 + 5000)));

        assertEquals("10,806 seconds", m_format.format(new Long(
                3 * DateUtils.MILLIS_PER_HOUR + 5050)));
        assertEquals("432,068 seconds", m_format.format(new Long(5 * DateUtils.MILLIS_PER_DAY
                + DateUtils.MILLIS_PER_MINUTE + 7070)));
    }

    public void testFormatNoLabels() {
        m_format.setMaxField(MillisDurationFormat.SECONDS);
        assertEquals("", m_format.format(new Long(0)));
        assertEquals("01", m_format.format(new Long(1000)));

        assertEquals("05", m_format.format(new Long(5000)));
        assertEquals("06", m_format.format(new Long(5999)));

        assertEquals("65", m_format.format(new Long(60000 + 5000)));

        assertEquals("10,806", m_format.format(new Long(3 * DateUtils.MILLIS_PER_HOUR + 5050)));
        assertEquals("432,068", m_format.format(new Long(5 * DateUtils.MILLIS_PER_DAY
                + DateUtils.MILLIS_PER_MINUTE + 7070)));
    }

    public void testFormatSeparator() {
        m_format.setMaxField(MillisDurationFormat.HOURS);
        m_format.setSeparator(":");
        assertEquals("", m_format.format(new Long(0)));
        assertEquals("03:00:06", m_format.format(new Long(3 * DateUtils.MILLIS_PER_HOUR + 5050)));
        assertEquals("03:01:00", m_format.format(new Long(3 * DateUtils.MILLIS_PER_HOUR + 59050)));
    }
}
