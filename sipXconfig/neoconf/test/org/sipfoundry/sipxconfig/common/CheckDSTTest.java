/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.common;

import java.util.Calendar;
import java.util.Date;
import java.util.TimeZone;

import junit.framework.TestCase;

public class CheckDSTTest extends TestCase {

    private CheckDST m_checkDST;

    protected void setUp() throws Exception {
        m_checkDST = new CheckDST();
    }

    public void testFindDstChangeTimeNoChange() {
        TimeZone tz = TimeZone.getTimeZone("PST");
        Calendar calendar = Calendar.getInstance(tz);
        calendar.set(Calendar.YEAR, 2007);
        calendar.set(Calendar.MONTH, Calendar.JUNE);

        // no DST changes in June 2007 in PST
        assertNull(m_checkDST.findDstChangeTime(tz, calendar.getTime()));
    }

    public void testFindDstChangeTimeSpringForward() {
        TimeZone tz = TimeZone.getTimeZone("PST");
        Calendar calendar = Calendar.getInstance(tz);
        calendar.set(Calendar.YEAR, 2007);
        calendar.set(Calendar.MONTH, Calendar.MARCH);
        calendar.set(Calendar.DAY_OF_MONTH, 10);
        calendar.set(Calendar.HOUR_OF_DAY, 23);
        calendar.set(Calendar.MINUTE, 3);

        assertFalse(tz.inDaylightTime(calendar.getTime()));
        Date dstChangeTime = m_checkDST.findDstChangeTime(tz, calendar.getTime());
        // change in PST - March 11, 2007 at 2am
        assertNotNull(dstChangeTime);
        assertTrue(tz.inDaylightTime(dstChangeTime));

        calendar.setTime(dstChangeTime);
        assertEquals(2007, calendar.get(Calendar.YEAR));
        assertEquals(Calendar.MARCH, calendar.get(Calendar.MONTH));
        assertEquals(11, calendar.get(Calendar.DAY_OF_MONTH));
        assertEquals(3, calendar.get(Calendar.HOUR_OF_DAY));
        assertEquals(3, calendar.get(Calendar.MINUTE));
    }

    public void testFindDstChangeTimeFallBack() {
        TimeZone tz = TimeZone.getTimeZone("PST");
        Calendar calendar = Calendar.getInstance(tz);
        calendar.set(Calendar.YEAR, 2007);
        calendar.set(Calendar.MONTH, Calendar.NOVEMBER);
        calendar.set(Calendar.DAY_OF_MONTH, 3);
        calendar.set(Calendar.HOUR_OF_DAY, 15);
        calendar.set(Calendar.MINUTE, 7);

        assertTrue(tz.inDaylightTime(calendar.getTime()));
        Date dstChangeTime = m_checkDST.findDstChangeTime(tz, calendar.getTime());
        // change in PST - November 4, 2007 at 2am
        assertNotNull(dstChangeTime);
        assertFalse(tz.inDaylightTime(dstChangeTime));

        calendar.setTime(dstChangeTime);
        assertEquals(2007, calendar.get(Calendar.YEAR));
        assertEquals(Calendar.NOVEMBER, calendar.get(Calendar.MONTH));
        assertEquals(4, calendar.get(Calendar.DAY_OF_MONTH));
        assertEquals(1, calendar.get(Calendar.HOUR_OF_DAY));
        assertEquals(7, calendar.get(Calendar.MINUTE));
    }

    public void testFindDstChangeSameDay() {
        TimeZone tz = TimeZone.getTimeZone("PST");
        Calendar calendar = Calendar.getInstance(tz);
        calendar.set(Calendar.YEAR, 2007);
        calendar.set(Calendar.MONTH, Calendar.NOVEMBER);
        calendar.set(Calendar.DAY_OF_MONTH, 4);
        calendar.set(Calendar.HOUR_OF_DAY, 0);
        calendar.set(Calendar.MINUTE, 7);

        assertTrue(tz.inDaylightTime(calendar.getTime()));
        Date dstChangeTime = m_checkDST.findDstChangeTime(tz, calendar.getTime());
        // change in PST - November 4, 2007 at 2am
        assertNotNull(dstChangeTime);
        assertFalse(tz.inDaylightTime(dstChangeTime));

        calendar.setTime(dstChangeTime);
        assertEquals(2007, calendar.get(Calendar.YEAR));
        assertEquals(Calendar.NOVEMBER, calendar.get(Calendar.MONTH));
        assertEquals(4, calendar.get(Calendar.DAY_OF_MONTH));
        assertEquals(1, calendar.get(Calendar.HOUR_OF_DAY));
        assertEquals(7, calendar.get(Calendar.MINUTE));
    }
}
