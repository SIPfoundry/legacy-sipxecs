/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.dialplan.attendant;

import java.util.ArrayList;
import java.util.Calendar;
import java.util.List;
import java.util.TimeZone;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.admin.ScheduledDay;
import org.sipfoundry.sipxconfig.admin.dialplan.attendant.WorkingTime.Interval;
import org.sipfoundry.sipxconfig.admin.dialplan.attendant.WorkingTime.InvalidPeriodException;
import org.sipfoundry.sipxconfig.admin.dialplan.attendant.WorkingTime.OverlappingPeriodsException;
import org.sipfoundry.sipxconfig.admin.dialplan.attendant.WorkingTime.SameStartAndStopHoursException;
import org.sipfoundry.sipxconfig.admin.dialplan.attendant.WorkingTime.WorkingHours;

public class WorkingTimeTest extends TestCase {

    protected void setUp() throws Exception {
        super.setUp();
    }

    public void testGetStartTime() throws Exception {
        Calendar c = Calendar.getInstance(TimeZone.getTimeZone("GMT"));
        c.set(1970, Calendar.JANUARY, 1, 10, 14);
        WorkingHours hours = new WorkingHours();
        hours.setStart(c.getTime());
        assertEquals("10:14", hours.getStartTime());
        assertEquals(c.getTime(), hours.getStart());
    }

    public void testGetStopTime() throws Exception {
        Calendar c = Calendar.getInstance(TimeZone.getTimeZone("GMT"));
        c.set(1970, Calendar.JANUARY, 1);
        c.set(Calendar.HOUR, 6);
        c.set(Calendar.MINUTE, 16);
        c.set(Calendar.AM_PM, Calendar.PM);
        WorkingHours hours = new WorkingHours();
        hours.setStop(c.getTime());
        assertEquals("18:16", hours.getStopTime());
        assertEquals(c.getTime(), hours.getStop());
    }

    public void testInitWokingHours() {
        WorkingHours hours = new WorkingHours();
        assertEquals("09:00", hours.getStartTime());
        assertEquals("18:00", hours.getStopTime());
    }

    public void testInitWorkingTime() {
        WorkingTime wt = new WorkingTime();
        WorkingHours[] workingHours = wt.getWorkingHours();
        assertEquals(7, workingHours.length);
        assertTrue(workingHours[0].isEnabled());
        assertEquals(ScheduledDay.MONDAY, workingHours[0].getDay());
        assertTrue(workingHours[4].isEnabled());
        assertFalse(workingHours[5].isEnabled());
        assertFalse(workingHours[6].isEnabled());
        assertEquals(ScheduledDay.SUNDAY, workingHours[6].getDay());
    }

    public void testGeneralAddMinutesFromSunday() {
        Calendar c = Calendar.getInstance(TimeZone.getTimeZone("GMT"));
        c.set(1970, Calendar.JANUARY, 1);

        WorkingHours hours = new WorkingHours();
        hours.setDay(ScheduledDay.SUNDAY);

        c.set(Calendar.HOUR_OF_DAY, 1);
        c.set(Calendar.MINUTE, 3);
        hours.setStart(c.getTime());

        c.set(Calendar.HOUR_OF_DAY, 2);
        c.set(Calendar.MINUTE, 5);
        hours.setStop(c.getTime());

        List<Interval> minutes = new ArrayList<Interval>();
        hours.addMinutesFromSunday(minutes, 0);
        assertEquals(1, minutes.size());
        assertEquals(63, minutes.get(0).getStart());
        assertEquals(125, minutes.get(0).getStop());

        hours.setDay(ScheduledDay.TUESDAY);
        minutes.clear();
        hours.addMinutesFromSunday(minutes, 0);
        assertEquals(1, minutes.size());
        assertEquals(2 * 24 * 60 + 63, minutes.get(0).getStart());
        assertEquals(2 * 24 * 60 + 125, minutes.get(0).getStop());
    }

    public void testGeneralAddMinutesFromSundayWithTimezone() {
        Calendar c = Calendar.getInstance(TimeZone.getTimeZone("GMT"));
        c.set(1970, Calendar.JANUARY, 1);

        WorkingHours hours = new WorkingHours();
        hours.setDay(ScheduledDay.TUESDAY);

        c.set(Calendar.HOUR_OF_DAY, 1);
        c.set(Calendar.MINUTE, 3);
        hours.setStart(c.getTime());

        c.set(Calendar.HOUR_OF_DAY, 2);
        c.set(Calendar.MINUTE, 5);
        hours.setStop(c.getTime());

        List<Interval> minutes = new ArrayList<Interval>();
        hours.addMinutesFromSunday(minutes, 120);
        assertEquals(1, minutes.size());
        assertEquals(2 * 24 * 60 + 63 - 120, minutes.get(0).getStart());
        assertEquals(2 * 24 * 60 + 125 - 120, minutes.get(0).getStop());

        minutes.clear();
        hours.addMinutesFromSunday(minutes, -183);
        assertEquals(1, minutes.size());
        assertEquals(2 * 24 * 60 + 63 + 183, minutes.get(0).getStart());
        assertEquals(2 * 24 * 60 + 125 + 183, minutes.get(0).getStop());
    }

    public void testGeneralAddMinutesFromSundayRollback() {
        Calendar c = Calendar.getInstance(TimeZone.getTimeZone("GMT"));
        c.set(1970, Calendar.JANUARY, 1);

        WorkingHours hours = new WorkingHours();
        hours.setDay(ScheduledDay.SUNDAY);

        c.set(Calendar.HOUR_OF_DAY, 1);
        c.set(Calendar.MINUTE, 3);
        hours.setStart(c.getTime());

        c.set(Calendar.HOUR_OF_DAY, 2);
        c.set(Calendar.MINUTE, 5);
        hours.setStop(c.getTime());

        List<Interval> minutes = new ArrayList<Interval>();
        hours.addMinutesFromSunday(minutes, 180);
        assertEquals(1, minutes.size());
        assertEquals(63 + WorkingHours.MINUTES_PER_WEEK - 180, minutes.get(0).getStart());
        assertEquals(125 + WorkingHours.MINUTES_PER_WEEK - 180, minutes.get(0).getStop());

        hours.setDay(ScheduledDay.SATURDAY);
        c.set(Calendar.HOUR_OF_DAY, 20);
        c.set(Calendar.MINUTE, 3);
        hours.setStart(c.getTime());

        c.set(Calendar.HOUR_OF_DAY, 22);
        c.set(Calendar.MINUTE, 5);
        hours.setStop(c.getTime());

        minutes.clear();
        hours.addMinutesFromSunday(minutes, -360);
        assertEquals(1, minutes.size());
        assertEquals(6 * 24 * 60 + 20 * 60 + 3 + 360 - WorkingHours.MINUTES_PER_WEEK, minutes
                .get(0).getStart());
        assertEquals(6 * 24 * 60 + 22 * 60 + 5 + 360 - WorkingHours.MINUTES_PER_WEEK, minutes
                .get(0).getStop());
    }

    public void testGeneralAddMinutesFromSundayRollbackSplit() {
        Calendar c = Calendar.getInstance(TimeZone.getTimeZone("GMT"));
        c.set(1970, Calendar.JANUARY, 1);

        WorkingHours hours = new WorkingHours();
        hours.setDay(ScheduledDay.SUNDAY);

        c.set(Calendar.HOUR_OF_DAY, 1);
        c.set(Calendar.MINUTE, 3);
        hours.setStart(c.getTime());

        c.set(Calendar.HOUR_OF_DAY, 2);
        c.set(Calendar.MINUTE, 5);
        hours.setStop(c.getTime());

        List<Interval> minutes = new ArrayList<Interval>();
        hours.addMinutesFromSunday(minutes, 80);
        assertEquals(2, minutes.size());
        assertEquals(63 + WorkingHours.MINUTES_PER_WEEK - 80, minutes.get(0).getStart());
        assertEquals(WorkingHours.MINUTES_PER_WEEK, minutes.get(0).getStop());
        assertEquals(0, minutes.get(1).getStart());
        assertEquals(125 - 80, minutes.get(1).getStop());

        hours.setDay(ScheduledDay.SATURDAY);
        c.set(Calendar.HOUR_OF_DAY, 20);
        c.set(Calendar.MINUTE, 3);
        hours.setStart(c.getTime());

        c.set(Calendar.HOUR_OF_DAY, 22);
        c.set(Calendar.MINUTE, 5);
        hours.setStop(c.getTime());

        minutes.clear();
        hours.addMinutesFromSunday(minutes, -130);
        assertEquals(2, minutes.size());
        assertEquals(6 * 24 * 60 + 20 * 60 + 3 + 130, minutes.get(0).getStart());
        assertEquals(WorkingHours.MINUTES_PER_WEEK, minutes.get(0).getStop());
        assertEquals(0, minutes.get(1).getStart());
        assertEquals(6 * 24 * 60 + 22 * 60 + 5 + 130 - WorkingHours.MINUTES_PER_WEEK, minutes
                .get(1).getStop());
    }

    public void testGeneralAddMinutesFromSundayWithEveryDaySchedule() {
        Calendar c = Calendar.getInstance(TimeZone.getTimeZone("GMT"));
        c.set(1970, Calendar.JANUARY, 1);

        WorkingHours hours = new WorkingHours();
        hours.setDay(ScheduledDay.EVERYDAY);

        c.set(Calendar.HOUR_OF_DAY, 1);
        c.set(Calendar.MINUTE, 3);
        hours.setStart(c.getTime());

        c.set(Calendar.HOUR_OF_DAY, 2);
        c.set(Calendar.MINUTE, 5);
        hours.setStop(c.getTime());

        List<Interval> minutes = new ArrayList<Interval>();
        hours.addMinutesFromSunday(minutes, 0);
        assertEquals(7, minutes.size());
        for (int i = 0; i < 7; i++) {
            assertEquals(i * 24 * 60 + 63, minutes.get(i).getStart());
            assertEquals(i * 24 * 60 + 125, minutes.get(i).getStop());
        }
    }

    public void testGeneralAddMinutesFromSundayWithWeekDaysSchedule() {
        Calendar c = Calendar.getInstance(TimeZone.getTimeZone("GMT"));
        c.set(1970, Calendar.JANUARY, 1);

        WorkingHours hours = new WorkingHours();
        hours.setDay(ScheduledDay.WEEKDAYS);

        c.set(Calendar.HOUR_OF_DAY, 1);
        c.set(Calendar.MINUTE, 3);
        hours.setStart(c.getTime());

        c.set(Calendar.HOUR_OF_DAY, 2);
        c.set(Calendar.MINUTE, 5);
        hours.setStop(c.getTime());

        List<Interval> minutes = new ArrayList<Interval>();
        hours.addMinutesFromSunday(minutes, 0);
        assertEquals(5, minutes.size());
        for (int i = 0; i < 5; i++) {
            assertEquals((i + 1) * 24 * 60 + 63, minutes.get(i).getStart());
            assertEquals((i + 1) * 24 * 60 + 125, minutes.get(i).getStop());
        }
    }

    public void testGeneralAddMinutesFromSundayWithWeekEndSchedule() {
        Calendar c = Calendar.getInstance(TimeZone.getTimeZone("GMT"));
        c.set(1970, Calendar.JANUARY, 1);

        WorkingHours hours = new WorkingHours();
        hours.setDay(ScheduledDay.WEEKEND);

        c.set(Calendar.HOUR_OF_DAY, 1);
        c.set(Calendar.MINUTE, 3);
        hours.setStart(c.getTime());

        c.set(Calendar.HOUR_OF_DAY, 2);
        c.set(Calendar.MINUTE, 5);
        hours.setStop(c.getTime());

        List<Interval> minutes = new ArrayList<Interval>();
        hours.addMinutesFromSunday(minutes, 0);
        assertEquals(2, minutes.size());
        assertEquals(6 * 24 * 60 + 63, minutes.get(0).getStart());
        assertEquals(6 * 24 * 60 + 125, minutes.get(0).getStop());
        assertEquals(0 * 24 * 60 + 63, minutes.get(1).getStart());
        assertEquals(0 * 24 * 60 + 125, minutes.get(1).getStop());
    }

    public void testInvalidPeriod() {
        Calendar c = Calendar.getInstance(TimeZone.getTimeZone("GMT"));
        c.set(1970, Calendar.JANUARY, 1);

        WorkingHours hours = new WorkingHours();
        hours.setDay(ScheduledDay.SUNDAY);

        c.set(Calendar.HOUR_OF_DAY, 3);
        c.set(Calendar.MINUTE, 3);
        hours.setStart(c.getTime());

        c.set(Calendar.HOUR_OF_DAY, 2);
        c.set(Calendar.MINUTE, 5);
        hours.setStop(c.getTime());

        assertTrue(hours.isInvalidPeriod());

        c.set(Calendar.HOUR_OF_DAY, 2);
        c.set(Calendar.MINUTE, 15);
        hours.setStart(c.getTime());

        c.set(Calendar.HOUR_OF_DAY, 6);
        c.set(Calendar.MINUTE, 50);
        hours.setStop(c.getTime());

        assertFalse(hours.isInvalidPeriod());
    }

    public void testSameHours() {
        Calendar c = Calendar.getInstance(TimeZone.getTimeZone("GMT"));
        c.set(1970, Calendar.JANUARY, 1);

        WorkingHours hours = new WorkingHours();
        hours.setDay(ScheduledDay.SUNDAY);

        c.set(Calendar.HOUR_OF_DAY, 3);
        c.set(Calendar.MINUTE, 3);
        hours.setStart(c.getTime());

        c.set(Calendar.HOUR_OF_DAY, 3);
        c.set(Calendar.MINUTE, 3);
        hours.setStop(c.getTime());

        assertTrue(hours.isTheSameHour());

        c.set(Calendar.HOUR_OF_DAY, 12);
        c.set(Calendar.MINUTE, 30);
        hours.setStart(c.getTime());

        c.set(Calendar.HOUR_OF_DAY, 18);
        c.set(Calendar.MINUTE, 40);
        hours.setStop(c.getTime());

        assertFalse(hours.isTheSameHour());
    }

    public void testOverlappingPeriods() {
        Calendar c = Calendar.getInstance(TimeZone.getTimeZone("GMT"));
        c.set(1970, Calendar.JANUARY, 1);

        WorkingTime wt = new WorkingTime();
        assertFalse(wt.overlappingPeriods());

        WorkingHours[] hours = new WorkingHours[2];
        hours[0] = new WorkingHours();
        hours[1] = new WorkingHours();
        wt.setWorkingHours(hours);

        hours[0].setDay(ScheduledDay.SUNDAY);
        hours[1].setDay(ScheduledDay.SUNDAY);

        // First schedule : SUNDAY, 03:03 -> 15:10
        // Second schedule : SUNDAY, 12:00 -> 17:23
        c.set(Calendar.HOUR_OF_DAY, 3);
        c.set(Calendar.MINUTE, 3);
        hours[0].setStart(c.getTime());

        c.set(Calendar.HOUR_OF_DAY, 15);
        c.set(Calendar.MINUTE, 10);
        hours[0].setStop(c.getTime());

        c.set(Calendar.HOUR_OF_DAY, 12);
        c.set(Calendar.MINUTE, 0);
        hours[1].setStart(c.getTime());

        c.set(Calendar.HOUR_OF_DAY, 17);
        c.set(Calendar.MINUTE, 23);
        hours[1].setStop(c.getTime());

        assertTrue(wt.overlappingPeriods());

        // First schedule : SUNDAY, 03:03 -> 15:10
        // Second schedule : SUNDAY, 01:00 -> 08:23
        // First schedule : SUNDAY, 02:15 -> 17:10
        // Second schedule : SUNDAY, 08:00 -> 10:23
        c.set(Calendar.HOUR_OF_DAY, 2);
        c.set(Calendar.MINUTE, 15);
        hours[0].setStart(c.getTime());

        c.set(Calendar.HOUR_OF_DAY, 17);
        c.set(Calendar.MINUTE, 10);
        hours[0].setStop(c.getTime());

        c.set(Calendar.HOUR_OF_DAY, 8);
        c.set(Calendar.MINUTE, 0);
        hours[1].setStart(c.getTime());

        c.set(Calendar.HOUR_OF_DAY, 10);
        c.set(Calendar.MINUTE, 23);
        hours[1].setStop(c.getTime());

        wt.setWorkingHours(hours);

        assertTrue(wt.overlappingPeriods());

        // First schedule : SUNDAY, 08:13 -> 10:10
        // Second schedule : SUNDAY, 03:00 -> 17:23
        c.set(Calendar.HOUR_OF_DAY, 8);
        c.set(Calendar.MINUTE, 13);
        hours[0].setStart(c.getTime());

        c.set(Calendar.HOUR_OF_DAY, 10);
        c.set(Calendar.MINUTE, 10);
        hours[0].setStop(c.getTime());

        c.set(Calendar.HOUR_OF_DAY, 3);
        c.set(Calendar.MINUTE, 0);
        hours[1].setStart(c.getTime());

        c.set(Calendar.HOUR_OF_DAY, 17);
        c.set(Calendar.MINUTE, 23);
        hours[1].setStop(c.getTime());

        wt.setWorkingHours(hours);

        assertTrue(wt.overlappingPeriods());

        // First schedule : SUNDAY, 03:03 -> 15:00
        // Second schedule : SUNDAY, 15:00 -> 20:23
        c.set(Calendar.HOUR_OF_DAY, 3);
        c.set(Calendar.MINUTE, 3);
        hours[0].setStart(c.getTime());

        c.set(Calendar.HOUR_OF_DAY, 15);
        c.set(Calendar.MINUTE, 10);
        hours[0].setStop(c.getTime());

        c.set(Calendar.HOUR_OF_DAY, 1);
        c.set(Calendar.MINUTE, 0);
        hours[1].setStart(c.getTime());

        c.set(Calendar.HOUR_OF_DAY, 8);
        c.set(Calendar.MINUTE, 23);
        hours[1].setStop(c.getTime());

        wt.setWorkingHours(hours);

        assertTrue(wt.overlappingPeriods());

        c.set(Calendar.HOUR_OF_DAY, 3);
        c.set(Calendar.MINUTE, 3);
        hours[0].setStart(c.getTime());

        c.set(Calendar.HOUR_OF_DAY, 15);
        c.set(Calendar.MINUTE, 0);
        hours[0].setStop(c.getTime());

        c.set(Calendar.HOUR_OF_DAY, 15);
        c.set(Calendar.MINUTE, 0);
        hours[1].setStart(c.getTime());

        c.set(Calendar.HOUR_OF_DAY, 20);
        c.set(Calendar.MINUTE, 23);
        hours[1].setStop(c.getTime());

        wt.setWorkingHours(hours);

        assertFalse(wt.overlappingPeriods());

        // First schedule : SUNDAY, 03:03 -> 15:00
        // Second schedule : SUNDAY, 01:00 -> 03:03
        // First schedule : SUNDAY, 03:03 -> 15:00
        // Second schedule : SUNDAY, 18:00 -> 20:23
        c.set(Calendar.HOUR_OF_DAY, 3);
        c.set(Calendar.MINUTE, 3);
        hours[0].setStart(c.getTime());

        c.set(Calendar.HOUR_OF_DAY, 15);
        c.set(Calendar.MINUTE, 0);
        hours[0].setStop(c.getTime());

        c.set(Calendar.HOUR_OF_DAY, 1);
        c.set(Calendar.MINUTE, 0);
        hours[1].setStart(c.getTime());

        c.set(Calendar.HOUR_OF_DAY, 3);
        c.set(Calendar.MINUTE, 3);
        hours[1].setStop(c.getTime());

        wt.setWorkingHours(hours);

        assertFalse(wt.overlappingPeriods());

        c.set(Calendar.HOUR_OF_DAY, 3);
        c.set(Calendar.MINUTE, 3);
        hours[0].setStart(c.getTime());

        c.set(Calendar.HOUR_OF_DAY, 15);
        c.set(Calendar.MINUTE, 0);
        hours[0].setStop(c.getTime());

        c.set(Calendar.HOUR_OF_DAY, 18);
        c.set(Calendar.MINUTE, 0);
        hours[1].setStart(c.getTime());

        c.set(Calendar.HOUR_OF_DAY, 20);
        c.set(Calendar.MINUTE, 23);
        hours[1].setStop(c.getTime());

        wt.setWorkingHours(hours);

        assertFalse(wt.overlappingPeriods());

        // First schedule : SUNDAY, 03:03 -> 15:00
        // Second schedule : MONDAY, 08:00 -> 14:23
        hours[0].setDay(ScheduledDay.SUNDAY);
        hours[1].setDay(ScheduledDay.MONDAY);

        c.set(Calendar.HOUR_OF_DAY, 3);
        c.set(Calendar.MINUTE, 3);
        hours[0].setStart(c.getTime());

        c.set(Calendar.HOUR_OF_DAY, 15);
        c.set(Calendar.MINUTE, 0);
        hours[0].setStop(c.getTime());

        c.set(Calendar.HOUR_OF_DAY, 8);
        c.set(Calendar.MINUTE, 0);
        hours[1].setStart(c.getTime());

        c.set(Calendar.HOUR_OF_DAY, 14);
        c.set(Calendar.MINUTE, 23);
        hours[1].setStop(c.getTime());

        assertFalse(wt.overlappingPeriods());
    }

    public void testCheckValid() {
        WorkingTime wt = new WorkingTime();

        Calendar c = Calendar.getInstance(TimeZone.getTimeZone("GMT"));
        c.set(1970, Calendar.JANUARY, 1);

        WorkingHours[] hours = new WorkingHours[1];
        hours[0] = new WorkingHours();

        hours[0].setDay(ScheduledDay.SUNDAY);

        c.set(Calendar.HOUR_OF_DAY, 3);
        c.set(Calendar.MINUTE, 3);
        hours[0].setStart(c.getTime());

        c.set(Calendar.HOUR_OF_DAY, 2);
        c.set(Calendar.MINUTE, 5);
        hours[0].setStop(c.getTime());

        wt.setWorkingHours(hours);

        try {
            wt.checkValid();
            fail("Should throw a InvalidPeriodException");
        } catch (InvalidPeriodException ex) {
            assertTrue(true);
        }

        c.set(Calendar.HOUR_OF_DAY, 3);
        c.set(Calendar.MINUTE, 3);
        hours[0].setStart(c.getTime());

        c.set(Calendar.HOUR_OF_DAY, 6);
        c.set(Calendar.MINUTE, 5);
        hours[0].setStop(c.getTime());

        wt.setWorkingHours(hours);

        try {
            wt.checkValid();
            assertTrue(true);
        } catch (InvalidPeriodException ex) {
            fail("Shouldn't throw a InvalidPeriodException");
        }

        c.set(Calendar.HOUR_OF_DAY, 3);
        c.set(Calendar.MINUTE, 3);
        hours[0].setStart(c.getTime());

        c.set(Calendar.HOUR_OF_DAY, 3);
        c.set(Calendar.MINUTE, 3);
        hours[0].setStop(c.getTime());

        try {
            wt.checkValid();
            fail("Should throw a SameStartAndStopHoursException");
        } catch (SameStartAndStopHoursException ex) {
            assertTrue(true);
        }

        c.set(Calendar.HOUR_OF_DAY, 12);
        c.set(Calendar.MINUTE, 30);
        hours[0].setStart(c.getTime());

        c.set(Calendar.HOUR_OF_DAY, 18);
        c.set(Calendar.MINUTE, 40);
        hours[0].setStop(c.getTime());

        try {
            wt.checkValid();
            assertTrue(true);
        } catch (SameStartAndStopHoursException ex) {
            fail("Shouldn't throw a SameStartAndStopHoursException");
        }

        hours = new WorkingHours[2];
        hours[0] = new WorkingHours();
        hours[1] = new WorkingHours();

        hours[0].setDay(ScheduledDay.SUNDAY);
        hours[1].setDay(ScheduledDay.SUNDAY);

        c.set(Calendar.HOUR_OF_DAY, 3);
        c.set(Calendar.MINUTE, 3);
        hours[0].setStart(c.getTime());

        c.set(Calendar.HOUR_OF_DAY, 15);
        c.set(Calendar.MINUTE, 10);
        hours[0].setStop(c.getTime());

        c.set(Calendar.HOUR_OF_DAY, 12);
        c.set(Calendar.MINUTE, 0);
        hours[1].setStart(c.getTime());

        c.set(Calendar.HOUR_OF_DAY, 17);
        c.set(Calendar.MINUTE, 23);
        hours[1].setStop(c.getTime());

        wt.setWorkingHours(hours);

        try {
            wt.checkValid();
            fail("Should throw a OverlappingPeriodsException");
        } catch (OverlappingPeriodsException ex) {
            assertTrue(true);
        }

        c.set(Calendar.HOUR_OF_DAY, 3);
        c.set(Calendar.MINUTE, 3);
        hours[0].setStart(c.getTime());

        c.set(Calendar.HOUR_OF_DAY, 15);
        c.set(Calendar.MINUTE, 10);
        hours[0].setStop(c.getTime());

        c.set(Calendar.HOUR_OF_DAY, 1);
        c.set(Calendar.MINUTE, 0);
        hours[1].setStart(c.getTime());

        c.set(Calendar.HOUR_OF_DAY, 8);
        c.set(Calendar.MINUTE, 23);
        hours[1].setStop(c.getTime());

        wt.setWorkingHours(hours);

        try {
            wt.checkValid();
            fail("Should throw a OverlappingPeriodsException");
        } catch (OverlappingPeriodsException ex) {
            assertTrue(true);
        }

        c.set(Calendar.HOUR_OF_DAY, 3);
        c.set(Calendar.MINUTE, 3);
        hours[0].setStart(c.getTime());

        c.set(Calendar.HOUR_OF_DAY, 15);
        c.set(Calendar.MINUTE, 0);
        hours[0].setStop(c.getTime());

        c.set(Calendar.HOUR_OF_DAY, 15);
        c.set(Calendar.MINUTE, 0);
        hours[1].setStart(c.getTime());

        c.set(Calendar.HOUR_OF_DAY, 20);
        c.set(Calendar.MINUTE, 23);
        hours[1].setStop(c.getTime());

        wt.setWorkingHours(hours);

        try {
            wt.checkValid();
            assertTrue(true);
        } catch (OverlappingPeriodsException ex) {
            fail("Shouldn't throw a OverlappingPeriodsException");
        }

        c.set(Calendar.HOUR_OF_DAY, 3);
        c.set(Calendar.MINUTE, 3);
        hours[0].setStart(c.getTime());

        c.set(Calendar.HOUR_OF_DAY, 15);
        c.set(Calendar.MINUTE, 0);
        hours[0].setStop(c.getTime());

        c.set(Calendar.HOUR_OF_DAY, 1);
        c.set(Calendar.MINUTE, 0);
        hours[1].setStart(c.getTime());

        c.set(Calendar.HOUR_OF_DAY, 3);
        c.set(Calendar.MINUTE, 3);
        hours[1].setStop(c.getTime());

        wt.setWorkingHours(hours);

        try {
            wt.checkValid();
            assertTrue(true);
        } catch (OverlappingPeriodsException ex) {
            fail("Shouldn't throw a OverlappingPeriodsException");
        }

        c.set(Calendar.HOUR_OF_DAY, 3);
        c.set(Calendar.MINUTE, 3);
        hours[0].setStart(c.getTime());

        c.set(Calendar.HOUR_OF_DAY, 15);
        c.set(Calendar.MINUTE, 0);
        hours[0].setStop(c.getTime());

        c.set(Calendar.HOUR_OF_DAY, 18);
        c.set(Calendar.MINUTE, 0);
        hours[1].setStart(c.getTime());

        c.set(Calendar.HOUR_OF_DAY, 20);
        c.set(Calendar.MINUTE, 23);
        hours[1].setStop(c.getTime());

        wt.setWorkingHours(hours);

        try {
            wt.checkValid();
            assertTrue(true);
        } catch (OverlappingPeriodsException ex) {
            fail("Shouldn't throw a OverlappingPeriodsException");
        }

        hours[0].setDay(ScheduledDay.SUNDAY);
        hours[1].setDay(ScheduledDay.MONDAY);

        c.set(Calendar.HOUR_OF_DAY, 3);
        c.set(Calendar.MINUTE, 3);
        hours[0].setStart(c.getTime());

        c.set(Calendar.HOUR_OF_DAY, 15);
        c.set(Calendar.MINUTE, 0);
        hours[0].setStop(c.getTime());

        c.set(Calendar.HOUR_OF_DAY, 8);
        c.set(Calendar.MINUTE, 0);
        hours[1].setStart(c.getTime());

        c.set(Calendar.HOUR_OF_DAY, 14);
        c.set(Calendar.MINUTE, 23);
        hours[1].setStop(c.getTime());

        try {
            wt.checkValid();
            assertTrue(true);
        } catch (OverlappingPeriodsException ex) {
            fail("Shouldn't throw a OverlappingPeriodsException");
        }
    }
}
