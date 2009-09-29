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

import java.util.Calendar;

import junit.framework.TestCase;

import org.apache.commons.lang.time.DateUtils;

public class CronScheduleTest extends TestCase {

    protected void setUp() throws Exception {
        super.setUp();
    }

/*
    public void testWeekly() {
        CronSchedule schedule = new CronSchedule();

        Calendar calendar = Calendar.getInstance();
        calendar.add(Calendar.DATE, 1);

        schedule.setType(CronSchedule.Type.WEEKLY);
        schedule.setDayOfWeek(calendar.get(Calendar.DAY_OF_WEEK));
        schedule.setHrs(10);
        schedule.setMin(30);

        calendar.set(Calendar.HOUR_OF_DAY, 10);
        calendar.set(Calendar.MINUTE, 30);

        assertEquals(7 * DateUtils.MILLIS_PER_DAY, schedule.getDelay());

        calendar = DateUtils.round(calendar, Calendar.MINUTE);

        assertEquals(calendar.getTime(), schedule.getFirstDate());
    }
*/
    public void testDaily() {
        CronSchedule schedule = new CronSchedule();

        Calendar calendar = Calendar.getInstance();
        calendar.add(Calendar.HOUR_OF_DAY, 1);
        calendar.set(Calendar.MINUTE, 30);

        schedule.setType(CronSchedule.Type.DAILY);

        schedule.setHrs(calendar.get(Calendar.HOUR_OF_DAY));
        schedule.setMin(30);

        assertEquals(DateUtils.MILLIS_PER_DAY, schedule.getDelay());

        calendar = DateUtils.round(calendar, Calendar.MINUTE);

        assertEquals(calendar.getTime(), schedule.getFirstDate());
    }

    public void testHourly() {
        CronSchedule schedule = new CronSchedule();

        Calendar calendar = Calendar.getInstance();
        calendar.add(Calendar.MINUTE, 10);

        schedule.setType(CronSchedule.Type.HOURLY);

        schedule.setMin(calendar.get(Calendar.MINUTE));

        assertEquals(DateUtils.MILLIS_PER_HOUR, schedule.getDelay());

        calendar = DateUtils.round(calendar, Calendar.MINUTE);

        assertEquals(calendar.getTime(), schedule.getFirstDate());
    }

    public void testSetCronStringHourly() {
        CronSchedule schedule = new CronSchedule();
        schedule.setCronString("0 25 * ? * *");

        assertEquals(CronSchedule.Type.HOURLY, schedule.getType());
        assertEquals(25, schedule.getMin());
        assertEquals(0, schedule.getHrs());
        assertEquals(0, schedule.getDayOfWeek());
    }

    public void testSetCronStringDaily() {
        CronSchedule schedule = new CronSchedule();
        schedule.setCronString("0 30 7 ? * *");

        assertEquals(CronSchedule.Type.DAILY, schedule.getType());
        assertEquals(30, schedule.getMin());
        assertEquals(7, schedule.getHrs());
        assertEquals(0, schedule.getDayOfWeek());
        assertEquals(ScheduledDay.EVERYDAY, schedule.getScheduledDay());
    }

    public void testSetCronStringWeekly() {
        CronSchedule schedule = new CronSchedule();
        schedule.setCronString("0 12 13 ? * 3");

        assertEquals(CronSchedule.Type.WEEKLY, schedule.getType());
        assertEquals(12, schedule.getMin());
        assertEquals(13, schedule.getHrs());
        assertEquals(3, schedule.getDayOfWeek());
    }

    public void testGetCronStringHourly() {
        CronSchedule schedule = new CronSchedule();
        schedule.setMin(25);
        schedule.setType(CronSchedule.Type.HOURLY);

        assertEquals("0 25 * ? * *", schedule.getCronString());
    }

    public void testGetCronStringDaily() {
        CronSchedule schedule = new CronSchedule();
        assertEquals("0 0 0 ? * *", schedule.getCronString());

        schedule.setMin(30);
        schedule.setHrs(7);
        schedule.setType(CronSchedule.Type.DAILY);

        assertEquals("0 30 7 ? * *", schedule.getCronString());
    }

    public void testGetCronStringWeekly() {
        CronSchedule schedule = new CronSchedule();
        schedule.setMin(12);
        schedule.setHrs(13);
        schedule.setDayOfWeek(Calendar.TUESDAY);
        schedule.setType(CronSchedule.Type.WEEKLY);

        assertEquals("0 12 13 ? * 3", schedule.getCronString());
    }

    public void testSetTimeOfDay() throws Exception {
        TimeOfDay timeOfDay = new TimeOfDay(10, 15);
        CronSchedule schedule = new CronSchedule();
        schedule.setTimeOfDay(timeOfDay);

        assertEquals(10, schedule.getHrs());
        assertEquals(15, schedule.getMin());
    }

    public void testGetTimeOfDay() throws Exception {
        CronSchedule schedule = new CronSchedule();
        schedule.setHrs(12);
        schedule.setMin(13);
        TimeOfDay timeOfDay = schedule.getTimeOfDay();
        assertEquals(12, timeOfDay.getHrs());
        assertEquals(13, timeOfDay.getMin());
    }

    public void testGetScheduledDay() {
        CronSchedule schedule = new CronSchedule();
        assertEquals(ScheduledDay.MONDAY, schedule.getScheduledDay());

        schedule.setDayOfWeek(Calendar.THURSDAY);
        assertEquals(ScheduledDay.THURSDAY, schedule.getScheduledDay());
    }

    public void testSetScheduledDay() {
        CronSchedule schedule = new CronSchedule();
        schedule.setScheduledDay(ScheduledDay.WEDNESDAY);
        assertEquals(Calendar.WEDNESDAY, schedule.getDayOfWeek());
    }
}
