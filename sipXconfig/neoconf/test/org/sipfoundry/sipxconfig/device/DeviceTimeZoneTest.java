/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.device;

import java.util.TimeZone;

import junit.framework.TestCase;

public class DeviceTimeZoneTest extends TestCase {

    public void testGetOffset() {
        TimeZone tz = TimeZone.getTimeZone("Europe/Helsinki");
        DeviceTimeZone mytz = new DeviceTimeZone(tz);

        int ofs = mytz.getOffset();
        assertEquals(120, ofs);

        ofs = mytz.getDstSavings();
        assertEquals(60, ofs);
    }

    public void testDST() {
        TimeZone tz = TimeZone.getTimeZone("Europe/Helsinki");
        DeviceTimeZone mytz = new DeviceTimeZone(tz);

        int value = mytz.getStartDayOfWeek();
        assertEquals(1, value);

        value = mytz.getStartTime();
        assertEquals(3 * 60, value);

        value = mytz.getStopTime();
        assertEquals(4 * 60, value);

        value = mytz.getStartWeek();
        assertEquals(DeviceTimeZone.DST_LASTWEEK, value);

        value = mytz.getStopWeek();
        assertEquals(DeviceTimeZone.DST_LASTWEEK, value);
    }

    public void testNorthAmericaDST() {
        TimeZone tz = TimeZone.getTimeZone("America/New_York");
        DeviceTimeZone mytz = new DeviceTimeZone(tz);

        // Starting in 2007, most of the United States and Canada observe DST from the second
        // Sunday in March to the first Sunday in November
        assertEquals(1, mytz.getStartDayOfWeek());
        assertEquals(1, mytz.getStopDayOfWeek());

        assertEquals(120, mytz.getStartTime()); // 2 AM
        assertEquals(120, mytz.getStopTime());

        assertEquals(2, mytz.getStartWeek());
        assertEquals(1, mytz.getStopWeek());

        assertEquals(2, mytz.getStartMonth());
        assertEquals(10, mytz.getStopMonth());
    }
}
