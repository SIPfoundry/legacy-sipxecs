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
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;

import junit.framework.TestCase;

public class DailyBackupScheduleTest extends TestCase {

    private DailyBackupSchedule schedule;

    protected void setUp() {
        schedule = new DailyBackupSchedule();
    }

    /**
     * Converts time of day expressed in 24-hour clock of local time into localized string
     * expressed in GMT time. We only need this for Whacker which should be rewritten to use
     * CronSchedule.
     */
    public static final String simpleTimeOfDayToLocalizedGmt(String tod) throws ParseException {
        SimpleDateFormat format = new SimpleDateFormat("HH:mm");
        format.setTimeZone(DailyBackupSchedule.GMT);
        Date date = format.parse(tod);
        return DailyBackupSchedule.GMT_TIME_OF_DAY_FORMAT.format(date);
    }


    public void testGetTimeOfDay() throws Exception {
        Date midnight = schedule.getTime();
        String actual = DailyBackupSchedule.GMT_TIME_OF_DAY_FORMAT.format(midnight);
        String expected = simpleTimeOfDayToLocalizedGmt("00:00");
        assertEquals(expected, actual);

        assertEquals(0, schedule.getTime().getTime()); // midnight gmt
    }

    public void testGetTimerPeriod() {
        assertEquals(1000 * 60 * 60 * 24, schedule.getTimerPeriod());
        schedule.setScheduledDay(ScheduledDay.THURSDAY);
        assertEquals(1000 * 60 * 60 * 24 * 7, schedule.getTimerPeriod());
    }

    public void testGetTimerDate() {
        assertNotNull(schedule.getTimerDate());
        schedule.setScheduledDay(ScheduledDay.EVERYDAY);
        DateFormat localTimeFormat = DateFormat.getTimeInstance(DateFormat.LONG);
        Date date;
        Calendar midnightLocal = Calendar.getInstance();
        midnightLocal.set(Calendar.HOUR_OF_DAY, 0);
        midnightLocal.set(Calendar.MINUTE, 0);
        midnightLocal.set(Calendar.SECOND, 0);
        midnightLocal.set(Calendar.MILLISECOND, 0);
        date = midnightLocal.getTime();
        date.setTime(date.getTime() + DailyBackupSchedule.ONCE_A_DAY);
        midnightLocal.setTime(date);
        // midnight local time
        String expected = localTimeFormat.format(midnightLocal.getTime());
        String actual = localTimeFormat.format(schedule.getTimerDate());
        assertEquals(expected, actual);
    }
}
