/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.device;

import java.util.Calendar;
import java.util.TimeZone;

import org.sipfoundry.sipxconfig.IntegrationTestCase;

public class TimeZoneManagerImplTestIntegration extends IntegrationTestCase {
    private TimeZoneManager m_timeZoneManager;
    private TimeZone defaultTimeZone = TimeZone.getDefault();

    public void setTimeZoneManager(TimeZoneManager tzm) {
        m_timeZoneManager = tzm;
    }

    public void testInitializationEurope() throws Exception {
        TimeZone.setDefault(TimeZone.getTimeZone("Europe/Helsinki"));
        m_timeZoneManager.saveDefault();
        DeviceTimeZone dtz = m_timeZoneManager.getDeviceTimeZone();

        assertEquals(60, dtz.getDstSavings());
        assertEquals(120, dtz.getOffset());

        assertEquals(Calendar.MARCH, dtz.getStartMonth());
        assertEquals(DeviceTimeZone.DST_LASTWEEK, dtz.getStartWeek());
        assertEquals(Calendar.SUNDAY, dtz.getStartDayOfWeek());
        assertEquals(180, dtz.getStartTime());

        assertEquals(Calendar.OCTOBER, dtz.getStopMonth());
        assertEquals(DeviceTimeZone.DST_LASTWEEK, dtz.getStopWeek());
        assertEquals(Calendar.SUNDAY, dtz.getStopDayOfWeek());
        assertEquals(240, dtz.getStopTime());

        cleanUp();
    }

    public void testInitializationUS() throws Exception {
        TimeZone.setDefault(TimeZone.getTimeZone("America/New_York"));
        m_timeZoneManager.saveDefault();
        DeviceTimeZone dtz = m_timeZoneManager.getDeviceTimeZone();

        assertEquals(60, dtz.getDstSavings());
        assertEquals(-300, dtz.getOffset());

        assertEquals(Calendar.MARCH, dtz.getStartMonth());
        assertEquals(2, dtz.getStartWeek());
        assertEquals(Calendar.SUNDAY, dtz.getStartDayOfWeek());
        assertEquals(120, dtz.getStartTime());

        assertEquals(Calendar.NOVEMBER, dtz.getStopMonth());
        assertEquals(1, dtz.getStopWeek());
        assertEquals(Calendar.SUNDAY, dtz.getStopDayOfWeek());
        assertEquals(120, dtz.getStopTime());

        cleanUp();
    }

    public void cleanUp() {
        TimeZone.setDefault(defaultTimeZone);
    }
}
