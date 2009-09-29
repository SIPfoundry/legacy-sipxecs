/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */

package org.sipfoundry.sipxconfig.admin.forwarding;

import java.util.Calendar;
import java.util.Date;
import java.util.TimeZone;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.admin.ScheduledDay;
import org.sipfoundry.sipxconfig.admin.dialplan.attendant.WorkingTime;
import org.sipfoundry.sipxconfig.admin.dialplan.attendant.WorkingTime.WorkingHours;

public class ScheduleTest extends TestCase {
    private static final String COLON = ":";

    public void testCalculateValidTime() {
        Schedule sch = new UserSchedule();
        WorkingHours[] hours = new WorkingHours[1];
        WorkingTime wt = new WorkingTime();

        hours[0] = new WorkingHours();

        Calendar cal = Calendar.getInstance(TimeZone.getTimeZone("GMT"));
        cal.set(2006, Calendar.NOVEMBER, 30, 10, 00);
        hours[0].setStart(cal.getTime());

        Integer startHour = Integer.valueOf(cal.get(Calendar.HOUR_OF_DAY));
        Integer startMinute = Integer.valueOf(cal.get(Calendar.MINUTE));

        cal.set(2006, Calendar.NOVEMBER, 30, 11, 00);
        hours[0].setStop(cal.getTime());

        Integer stopHour = Integer.valueOf(cal.get(Calendar.HOUR_OF_DAY));
        Integer stopMinute = Integer.valueOf(cal.get(Calendar.MINUTE));

        hours[0].setEnabled(true);
        hours[0].setDay(ScheduledDay.WEDNESDAY);

        Integer minutesFromSunday = (hours[0].getDay().getDayOfWeek() - 1) * 24 * 60;

        wt.setWorkingHours(hours);
        wt.setEnabled(true);

        sch.setWorkingTime(wt);

        int tz_offset = TimeZone.getDefault().getOffset((new Date()).getTime()) / 60000;

        Integer startWithTimezone = minutesFromSunday + startHour * 60 + startMinute - tz_offset;
        Integer stopWithTimezone = minutesFromSunday + stopHour * 60 + stopMinute - tz_offset;

        String expected = Integer.toHexString(startWithTimezone) + COLON
                + Integer.toHexString(stopWithTimezone);
        String actual = sch.calculateValidTime();

        assertEquals(expected, actual);
    }

    public void testCalculateValidTimeSaturdayWithTwoPeriods() {
        TimeZone default_tz = TimeZone.getDefault();
        TimeZone.setDefault(TimeZone.getTimeZone("GMT-11"));
        Schedule sch = new UserSchedule();
        WorkingHours[] hours = new WorkingHours[1];
        WorkingTime wt = new WorkingTime();

        hours[0] = new WorkingHours();

        Calendar cal = Calendar.getInstance(TimeZone.getTimeZone("GMT"));
        cal.set(2006, Calendar.NOVEMBER, 30, 10, 00);
        hours[0].setStart(cal.getTime());

        Integer startHour = Integer.valueOf(cal.get(Calendar.HOUR_OF_DAY));
        Integer startMinute = Integer.valueOf(cal.get(Calendar.MINUTE));

        cal.set(2006, Calendar.NOVEMBER, 30, 23, 00);
        hours[0].setStop(cal.getTime());

        Integer stopHour = Integer.valueOf(cal.get(Calendar.HOUR_OF_DAY));
        Integer stopMinute = Integer.valueOf(cal.get(Calendar.MINUTE));

        hours[0].setEnabled(true);
        hours[0].setDay(ScheduledDay.SATURDAY);

        Integer minutesFromSunday = (hours[0].getDay().getDayOfWeek() - 1) * 24 * 60;

        wt.setWorkingHours(hours);
        wt.setEnabled(true);

        sch.setWorkingTime(wt);

        int tz_offset = TimeZone.getDefault().getOffset((new Date()).getTime()) / 60000;

        Integer startWithTimezone = minutesFromSunday + startHour * 60 + startMinute - tz_offset;
        Integer stopWithTimezone = minutesFromSunday + stopHour * 60 + stopMinute - tz_offset;

        String expected = Integer.toHexString(startWithTimezone) + COLON
                + Integer.toHexString(WorkingHours.MINUTES_PER_WEEK) + COLON + "0" + COLON
                + Integer.toHexString(stopWithTimezone - WorkingHours.MINUTES_PER_WEEK);
        String actual = sch.calculateValidTime();

        assertEquals(expected, actual);
        TimeZone.setDefault(default_tz);
    }

    public void testCalculateValidTimeSundayWithTwoPeriods() {
        TimeZone default_tz = TimeZone.getDefault();
        TimeZone.setDefault(TimeZone.getTimeZone("GMT+5"));
        Schedule sch = new UserSchedule();
        WorkingHours[] hours = new WorkingHours[1];
        WorkingTime wt = new WorkingTime();

        hours[0] = new WorkingHours();

        Calendar cal = Calendar.getInstance(TimeZone.getTimeZone("GMT"));
        cal.set(2006, Calendar.NOVEMBER, 30, 02, 00);
        hours[0].setStart(cal.getTime());

        Integer startHour = Integer.valueOf(cal.get(Calendar.HOUR_OF_DAY));
        Integer startMinute = Integer.valueOf(cal.get(Calendar.MINUTE));

        cal.set(2006, Calendar.NOVEMBER, 30, 15, 00);
        hours[0].setStop(cal.getTime());

        Integer stopHour = Integer.valueOf(cal.get(Calendar.HOUR_OF_DAY));
        Integer stopMinute = Integer.valueOf(cal.get(Calendar.MINUTE));

        hours[0].setEnabled(true);
        hours[0].setDay(ScheduledDay.SUNDAY);

        Integer minutesFromSunday = (hours[0].getDay().getDayOfWeek() - 1) * 24 * 60;

        wt.setWorkingHours(hours);
        wt.setEnabled(true);

        sch.setWorkingTime(wt);

        int tz_offset = TimeZone.getDefault().getOffset((new Date()).getTime()) / 60000;

        Integer startWithTimezone = minutesFromSunday + startHour * 60 + startMinute - tz_offset;
        Integer stopWithTimezone = minutesFromSunday + stopHour * 60 + stopMinute - tz_offset;

        String expected = Integer.toHexString(startWithTimezone + WorkingHours.MINUTES_PER_WEEK)
                + COLON + Integer.toHexString(WorkingHours.MINUTES_PER_WEEK) + COLON + "0" + COLON
                + Integer.toHexString(stopWithTimezone);
        String actual = sch.calculateValidTime();

        assertEquals(expected, actual);
        TimeZone.setDefault(default_tz);
    }
}
