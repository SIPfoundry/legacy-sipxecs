/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.dialplan.attendant;

import java.sql.Timestamp;
import java.util.Calendar;
import java.util.Date;
import java.util.GregorianCalendar;
import java.util.List;

import junit.framework.TestCase;

import org.sipfoundry.commons.util.HolidayPeriod;

public class HolidayTest extends TestCase {
    private HolidayPeriod m_now;
    private HolidayPeriod m_then;

    protected void setUp() throws Exception {
        m_now = new HolidayPeriod();
        m_now.setStartDate(new Date());
        m_then = new HolidayPeriod();
        m_then.setStartDate(new Timestamp(0));
    }

    public void testaddPeriod() {
        Holiday holiday = new Holiday();
        assertTrue(holiday.getPeriods().isEmpty());
        holiday.addPeriod(m_now);
        holiday.addPeriod(m_then);
        holiday.addPeriod(m_now);
        assertEquals(3, holiday.getPeriods().size());
        assertTrue(holiday.getPeriods().contains(m_now));
        assertTrue(holiday.getPeriods().contains(m_then));
    }

    public void testRemovePeriod() {
        Holiday holiday = new Holiday();
        holiday.addPeriod(m_now);
        assertEquals(1, holiday.getPeriods().size());
        holiday.removePeriod(m_then);
        assertEquals(1, holiday.getPeriods().size());
        holiday.removePeriod(m_now);
        assertTrue(holiday.getPeriods().isEmpty());
    }

    public void testNotNullPeriod() {
        Calendar calStart = new GregorianCalendar();
        Calendar calEnd = new GregorianCalendar();
        Calendar calStartToday = new GregorianCalendar();
        calStartToday.setTime(new Date());
        calStartToday.set(Calendar.HOUR_OF_DAY, 0);
        calStartToday.set(Calendar.MINUTE, 0);
        calStartToday.set(Calendar.SECOND, 0);
        Calendar calStartADay = new GregorianCalendar();
        calStartADay.set(Calendar.YEAR, 2014);
        calStartADay.set(Calendar.MONTH, 4);
        calStartADay.set(Calendar.DAY_OF_MONTH, 22);
        calStartADay.set(Calendar.HOUR_OF_DAY, 0);
        calStartADay.set(Calendar.MINUTE, 0);
        calStartADay.set(Calendar.SECOND, 0);
        //when start date is set to null, the period will default to 1 day holiday: today
        HolidayPeriod period = new HolidayPeriod();
        period.setStartDate(null);
        period.setEndDate(null);
        calStart.setTime(period.getStartDate());
        calEnd.setTime(period.getEndDate());
        assertEquals(calStart.get(Calendar.YEAR), calStartToday.get(Calendar.YEAR));
        assertEquals(calStart.get(Calendar.MONTH), calStartToday.get(Calendar.MONTH));
        assertEquals(calStart.get(Calendar.DAY_OF_MONTH), calStartToday.get(Calendar.DAY_OF_MONTH));
        assertEquals(calStart.get(Calendar.HOUR_OF_DAY), calStartToday.get(Calendar.HOUR_OF_DAY));
        assertEquals(calStart.get(Calendar.MINUTE), calStartToday.get(Calendar.MINUTE));
        assertEquals(calStart.get(Calendar.SECOND), calStartToday.get(Calendar.SECOND));
        assertEquals(calEnd.get(Calendar.YEAR), calStartToday.get(Calendar.YEAR));
        assertEquals(calEnd.get(Calendar.MONTH), calStartToday.get(Calendar.MONTH));
        assertEquals(calEnd.get(Calendar.DAY_OF_MONTH), calStartToday.get(Calendar.DAY_OF_MONTH) + 1);
        assertEquals(calEnd.get(Calendar.HOUR_OF_DAY), 0);
        assertEquals(calEnd.get(Calendar.MINUTE), 0);
        assertEquals(calEnd.get(Calendar.SECOND), 0);
        //when start date is set to a day not null, but end date is null,
        //the period will default to 1 day holiday (the day of start date)
        HolidayPeriod period2 = new HolidayPeriod();
        period2.setStartDate(calStartADay.getTime());
        period2.setEndDate(null);
        calStart.setTime(period2.getStartDate());
        calEnd.setTime(period2.getEndDate());
        assertEquals(calStart.get(Calendar.YEAR), 2014);
        assertEquals(calStart.get(Calendar.MONTH), 4);
        assertEquals(calStart.get(Calendar.DAY_OF_MONTH), 22);
        assertEquals(calStart.get(Calendar.HOUR_OF_DAY), 0);
        assertEquals(calStart.get(Calendar.MINUTE), 0);
        assertEquals(calStart.get(Calendar.SECOND), 0);
        assertEquals(calEnd.get(Calendar.YEAR), 2014);
        assertEquals(calEnd.get(Calendar.MONTH), 4);
        assertEquals(calEnd.get(Calendar.DAY_OF_MONTH), 23);
        assertEquals(calEnd.get(Calendar.HOUR_OF_DAY), 0);
        assertEquals(calEnd.get(Calendar.MINUTE), 0);
        assertEquals(calEnd.get(Calendar.SECOND), 0);

    }

    public void testGetDay() {
        Holiday holiday = new Holiday();
        assertTrue(holiday.getPeriods().isEmpty());
        for (int i = 0; i < 3; i++) {
            assertNotNull(holiday.getPeriod(i));
            assertEquals(i + 1, holiday.getPeriods().size());
        }
    }

    public void testChop() {
        HolidayPeriod[] holidayPeriods = {
            m_now, m_then, getNewHolidayPeriod(), getNewHolidayPeriod(), getNewHolidayPeriod()
        };
        Holiday holiday = new Holiday();
        assertTrue(holiday.getPeriods().isEmpty());
        for (int i = 0; i < holidayPeriods.length; i++) {
            holiday.addPeriod(holidayPeriods[i]);
        }
        assertEquals(5, holiday.getPeriods().size());
        holiday.chop(2);
        assertEquals(3, holiday.getPeriods().size());
        List days = holiday.getPeriods();
        for(int i = 0; i < days.size(); i++) {
            assertSame(holidayPeriods[i], days.get(i));
        }
    }

    public void testChopLast() {
        Holiday holiday = new Holiday();
        assertTrue(holiday.getPeriods().isEmpty());
        for (int i = 0; i < 3; i++) {
            holiday.addPeriod(getNewHolidayPeriod());
        }
        assertEquals(3, holiday.getPeriods().size());
        holiday.chop(2);
        assertEquals(3, holiday.getPeriods().size());
    }

    public void testChopOutOfRange() {
        Holiday holiday = new Holiday();
        assertTrue(holiday.getPeriods().isEmpty());
        for (int i = 0; i < 3; i++) {
            holiday.addPeriod(getNewHolidayPeriod());
        }
        assertEquals(3, holiday.getPeriods().size());
        holiday.chop(4);
        assertEquals(3, holiday.getPeriods().size());
    }

    private HolidayPeriod getNewHolidayPeriod() {
        HolidayPeriod holidayPeriod = new HolidayPeriod();
        holidayPeriod.setStartDate(new Date());
        return holidayPeriod;
    }
}
