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
import java.util.Date;
import java.util.List;

import org.sipfoundry.commons.util.HolidayPeriod;

import junit.framework.TestCase;

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
