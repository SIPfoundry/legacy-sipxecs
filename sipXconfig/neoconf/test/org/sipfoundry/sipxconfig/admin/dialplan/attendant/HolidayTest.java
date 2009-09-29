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

import java.sql.Timestamp;
import java.util.Date;
import java.util.List;

import junit.framework.TestCase;

public class HolidayTest extends TestCase {
    private Date m_now;
    private Date m_then;

    protected void setUp() throws Exception {
        m_now = new Date();
        m_then = new Timestamp(0);
    }

    public void testAddDay() {
        Holiday holiday = new Holiday();
        assertTrue(holiday.getDates().isEmpty());
        holiday.addDay(m_now);
        holiday.addDay(m_then);
        holiday.addDay(m_now);
        assertEquals(3, holiday.getDates().size());
        assertTrue(holiday.getDates().contains(m_now));
        assertTrue(holiday.getDates().contains(m_then));
    }

    public void testRemoveDay() {
        Holiday holiday = new Holiday();
        holiday.addDay(m_now);
        assertEquals(1, holiday.getDates().size());
        holiday.removeDay(m_then);
        assertEquals(1, holiday.getDates().size());
        holiday.removeDay(m_now);
        assertTrue(holiday.getDates().isEmpty());
    }

    public void testGetDay() {
        Holiday holiday = new Holiday();
        assertTrue(holiday.getDates().isEmpty());
        for (int i = 0; i < 3; i++) {
            assertNotNull(holiday.getDay(i));
            assertEquals(i + 1, holiday.getDates().size());
        }
    }

    public void testChop() {
        Date[] dates = {
            m_now, m_then, new Date(), new Date(), new Date()
        };
        Holiday holiday = new Holiday();
        assertTrue(holiday.getDates().isEmpty());
        for (int i = 0; i < dates.length; i++) {
            holiday.addDay(dates[i]);
        }
        assertEquals(5, holiday.getDates().size());
        holiday.chop(2);
        assertEquals(3, holiday.getDates().size());
        List days = holiday.getDates();
        for(int i = 0; i < days.size(); i++) {
            assertSame(dates[i], days.get(i));
        }
    }

    public void testChopLast() {
        Holiday holiday = new Holiday();
        assertTrue(holiday.getDates().isEmpty());
        for (int i = 0; i < 3; i++) {
            holiday.addDay(new Date());
        }
        assertEquals(3, holiday.getDates().size());
        holiday.chop(2);
        assertEquals(3, holiday.getDates().size());
    }

    public void testChopOutOfRange() {
        Holiday holiday = new Holiday();
        assertTrue(holiday.getDates().isEmpty());
        for (int i = 0; i < 3; i++) {
            holiday.addDay(new Date());
        }
        assertEquals(3, holiday.getDates().size());
        holiday.chop(4);
        assertEquals(3, holiday.getDates().size());
    }
}
