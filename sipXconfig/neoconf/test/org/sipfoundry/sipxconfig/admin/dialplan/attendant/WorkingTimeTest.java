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

    public void testAddMinutesFromSunday() {
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

        List<Integer> minutes = new ArrayList<Integer>();
        hours.addMinutesFromSunday(minutes, 0);
        assertEquals(2, minutes.size());
        assertEquals(63, minutes.get(0).intValue());
        assertEquals(125, minutes.get(1).intValue());

        hours.setDay(ScheduledDay.TUESDAY);
        minutes.clear();
        hours.addMinutesFromSunday(minutes, 0);
        assertEquals(2, minutes.size());
        assertEquals(2 * 24 * 60 + 63, minutes.get(0).intValue());
        assertEquals(2 * 24 * 60 + 125, minutes.get(1).intValue());
    }

    public void testAddMinutesFromSundayWithTimezone() {
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

        List<Integer> minutes = new ArrayList<Integer>();
        hours.addMinutesFromSunday(minutes, 120);
        assertEquals(2, minutes.size());
        assertEquals(2 * 24 * 60 + 63 - 120, minutes.get(0).intValue());
        assertEquals(2 * 24 * 60 + 125 - 120, minutes.get(1).intValue());

        minutes.clear();
        hours.addMinutesFromSunday(minutes, -183);
        assertEquals(2, minutes.size());
        assertEquals(2 * 24 * 60 + 63 + 183, minutes.get(0).intValue());
        assertEquals(2 * 24 * 60 + 125 + 183, minutes.get(1).intValue());
    }

    public void testAddMinutesFromSundayRollback() {
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

        List<Integer> minutes = new ArrayList<Integer>();
        hours.addMinutesFromSunday(minutes, 180);
        assertEquals(2, minutes.size());
        assertEquals(63 + WorkingHours.MINUTES_PER_WEEK - 180, minutes.get(0).intValue());
        assertEquals(125 + WorkingHours.MINUTES_PER_WEEK - 180, minutes.get(1).intValue());

        hours.setDay(ScheduledDay.SATURDAY);
        c.set(Calendar.HOUR_OF_DAY, 20);
        c.set(Calendar.MINUTE, 3);
        hours.setStart(c.getTime());

        c.set(Calendar.HOUR_OF_DAY, 22);
        c.set(Calendar.MINUTE, 5);
        hours.setStop(c.getTime());

        minutes.clear();
        hours.addMinutesFromSunday(minutes, -360);
        assertEquals(2, minutes.size());
        assertEquals(6 * 24 * 60 + 20 * 60 + 3 + 360 - WorkingHours.MINUTES_PER_WEEK, minutes.get(0)
                .intValue());
        assertEquals(6 * 24 * 60 + 22 * 60 + 5 + 360 - WorkingHours.MINUTES_PER_WEEK, minutes.get(1)
                .intValue());
    }

    public void testAddMinutesFromSundayRollbackSplit() {
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

        List<Integer> minutes = new ArrayList<Integer>();
        hours.addMinutesFromSunday(minutes, 80);
        assertEquals(4, minutes.size());
        assertEquals(63 + WorkingHours.MINUTES_PER_WEEK - 80, minutes.get(0).intValue());
        assertEquals(WorkingHours.MINUTES_PER_WEEK, minutes.get(1).intValue());
        assertEquals(0, minutes.get(2).intValue());
        assertEquals(125 - 80, minutes.get(3).intValue());

        hours.setDay(ScheduledDay.SATURDAY);
        c.set(Calendar.HOUR_OF_DAY, 20);
        c.set(Calendar.MINUTE, 3);
        hours.setStart(c.getTime());

        c.set(Calendar.HOUR_OF_DAY, 22);
        c.set(Calendar.MINUTE, 5);
        hours.setStop(c.getTime());

        minutes.clear();
        hours.addMinutesFromSunday(minutes, -130);
        assertEquals(4, minutes.size());
        assertEquals(6 * 24 * 60 + 20 * 60 + 3 + 130, minutes.get(0).intValue());
        assertEquals(WorkingHours.MINUTES_PER_WEEK, minutes.get(1).intValue());
        assertEquals(0, minutes.get(2).intValue());
        assertEquals(6 * 24 * 60 + 22 * 60 + 5 + 130 - WorkingHours.MINUTES_PER_WEEK, minutes.get(3)
                .intValue());
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

        WorkingHours[] hours = new WorkingHours[2];
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

        assertTrue(wt.overlappingPeriods());

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
