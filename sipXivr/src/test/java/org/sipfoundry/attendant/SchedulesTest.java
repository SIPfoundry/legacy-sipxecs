/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.attendant;

import java.text.DateFormat;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;

import org.sipfoundry.attendant.Schedule;
import org.sipfoundry.attendant.Schedule.Day;

import junit.framework.TestCase;

public class SchedulesTest extends TestCase {

    public void testGetAttendant1() {
        DateFormat dateFormat = new SimpleDateFormat("dd-MMM-yyyy");
        DateFormat dateTimeFormat = new SimpleDateFormat("dd-MMM-yyyy HH:mm");
        Schedule s = new Schedule();
        try {
            Date date = dateTimeFormat.parse("01-jan-2001 12:00");
            assertNull("empty Schedules didn't return null!", s.getAttendant(date));

            Date NewYears = dateFormat.parse("01-JAN-2009");
            Date Summer = dateFormat.parse("21-JUN-2009");
            s.getHolidays().setAttendantId("holiday");
            s.getHolidays().add(NewYears);
            s.getHolidays().add(Summer);

            assertNull("Holiday Schedules didn't return null!", s.getAttendant(date));

            date = dateTimeFormat.parse("31-DEC-2008 23:59");
            assertNull("Pre NewYears", s.getAttendant(date));
            date = dateTimeFormat.parse("01-JAN-2009 00:00");
            assertEquals("holiday", s.getAttendant(date));
            date = dateTimeFormat.parse("01-JAN-2009 23:59");
            assertEquals("holiday", s.getAttendant(date));
            date = dateTimeFormat.parse("02-JAN-2009 00:00");

            assertNull("Post NewYears", s.getAttendant(date));

            date = dateTimeFormat.parse("20-JUN-2009 23:59");
            assertNull("Pre Summer", s.getAttendant(date));
            date = dateTimeFormat.parse("21-JUN-2009 00:00");
            assertEquals("holiday", s.getAttendant(date));
            date = dateTimeFormat.parse("21-JUN-2009 12:00");
            assertEquals("holiday", s.getAttendant(date));
            date = dateTimeFormat.parse("21-JUN-2009 23:59");
            assertEquals("holiday", s.getAttendant(date));
            date = dateTimeFormat.parse("22-JUN-2009 00:00");
            assertNull("Post Summer", s.getAttendant(date));

        } catch (ParseException e) {
            fail(e.toString());
        }

    }

    public void testGetAttendant2() {
        DateFormat timeFormat = new SimpleDateFormat("HH:mm");
        DateFormat dateFormat = new SimpleDateFormat("dd-MMM-yyyy");
        DateFormat dateTimeFormat = new SimpleDateFormat("dd-MMM-yyyy HH:mm");
        Schedule s = new Schedule();
        try {
            s.getHours().setAfterHoursAttendantId("afterhours");
            s.getHours().setRegularHoursAttendantId("regularhours");
            s.getHolidays().setAttendantId("holiday");

            Day day = s.new Day();
            day.setRange(s.new TimeRange());
            day.setDayOfWeek(Calendar.MONDAY);
            day.getRange().setFrom(timeFormat.parse("09:00"));
            day.getRange().setTo(timeFormat.parse("17:00"));
            s.getHours().add(day);

            Date date = dateTimeFormat.parse("28-DEC-2008 23:59"); // A Sunday
            assertEquals("afterhours", s.getAttendant(date));
            date = dateTimeFormat.parse("29-DEC-2008 00:00"); // A Monday
            assertEquals("afterhours", s.getAttendant(date));
            date = dateTimeFormat.parse("29-DEC-2008 08:00"); // Not open yet
            assertEquals("afterhours", s.getAttendant(date));
            date = dateTimeFormat.parse("29-DEC-2008 09:00"); // Now open!
            assertEquals("regularhours", s.getAttendant(date));
            date = dateTimeFormat.parse("29-DEC-2008 12:00"); // Lunch time!
            assertEquals("regularhours", s.getAttendant(date));
            date = dateTimeFormat.parse("29-DEC-2008 16:59"); // About to close...
            assertEquals("regularhours", s.getAttendant(date));
            date = dateTimeFormat.parse("29-DEC-2008 17:00"); // Closed!
            assertEquals("afterhours", s.getAttendant(date));

            day = s.new Day();
            day.setRange(s.new TimeRange());
            day.setDayOfWeek(Calendar.WEDNESDAY);
            day.getRange().setFrom(timeFormat.parse("09:00"));
            day.getRange().setTo(timeFormat.parse("21:00"));
            s.getHours().add(day);
            day = s.new Day();
            day.setRange(s.new TimeRange());
            day.setDayOfWeek(Calendar.FRIDAY);
            day.getRange().setFrom(timeFormat.parse("06:00"));
            day.getRange().setTo(timeFormat.parse("12:00"));
            s.getHours().add(day);

            /*
             * January 1974 Su Mo Tu We Th Fr Sa 1 2 3 4 5 Holiday 12/12 0/24 6/18 0/24 18/78 6 7
             * 8 9 10 11 12 0/24 8/16 0/24 12/12 0/24 6/18 0/24 26/142 13 14 15 16 17 18 19 0/24
             * 8/16 0/24 12/12 0/24 6/18 0/24 26/142 20 21 22 23 24 25 26 0/24 8/16 0/24 12/12
             * 0/24 6/18 Holiday 26/118 27 28 29 30 31 0/24 8/16 0/24 12/12 0/ 20/100
             * 
             * Total: 48 Holidays, 116 regular, 580 afterhours
             * 
             */
            int holidays = 0;
            int afterhours = 0;
            int regularhours = 0;
            s.getHolidays().add(dateFormat.parse("01-JAN-1974"));
            s.getHolidays().add(dateFormat.parse("26-JAN-1974"));
            Calendar c = Calendar.getInstance();
            c.setTime(dateTimeFormat.parse("01-JAN-1974 00:00"));
            for (int i = 0; i < 31 * 24; i++) {
                String attendant = s.getAttendant(c.getTime());
                if (attendant.equals("holiday")) {
                    holidays++;
                } else if (attendant.equals("afterhours")) {
                    afterhours++;
                } else if (attendant.equals("regularhours")) {
                    regularhours++;
                }
                c.add(Calendar.HOUR_OF_DAY, 1);
            }
            assertEquals(48, holidays);
            assertEquals(116, regularhours);
            assertEquals(580, afterhours);

        } catch (ParseException e) {
            fail(e.toString());
        }

    }

}
