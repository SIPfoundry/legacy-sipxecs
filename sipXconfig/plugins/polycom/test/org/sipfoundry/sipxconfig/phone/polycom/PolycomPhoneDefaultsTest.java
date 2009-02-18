/*
 *
 *
 * Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.phone.polycom;

import java.lang.reflect.Method;
import java.util.TimeZone;

import junit.framework.TestCase;

import org.easymock.EasyMock;
import org.sipfoundry.sipxconfig.device.DeviceDefaults;
import org.sipfoundry.sipxconfig.device.DeviceTimeZone;
import org.sipfoundry.sipxconfig.device.TimeZoneManager;
import org.sipfoundry.sipxconfig.speeddial.SpeedDial;

public class PolycomPhoneDefaultsTest extends TestCase {

    private PolycomPhoneDefaults m_polycomPhoneDefaults;
    private DeviceTimeZone m_deviceTimeZone;

    protected void setUpWithSpecifiedTimeZone(String timezone) {
        // Create a fresh new DeviceTimeZone for each test case.
        setUpWithDeviceTimeZone(new DeviceTimeZone(TimeZone.getTimeZone(timezone)));
    }

    protected void setUpWithDeviceTimeZone(DeviceTimeZone deviceTimeZone) {
        // Record the fresh new DeviceTimeZone for each test case.
        m_deviceTimeZone = deviceTimeZone;

        // Mock TimeZoneManager::getDeviceTimeZone() to return our fresh new DeviceTimeZone.
        TimeZoneManager tzm = EasyMock.createNiceMock(TimeZoneManager.class);
        EasyMock.expect(tzm.getDeviceTimeZone()).andReturn(m_deviceTimeZone).atLeastOnce();
        EasyMock.replay(tzm);

        // Create a fresh new PolycomPhoneDefaults that will use our fresh new DeviceTimeZone.
        DeviceDefaults deviceDefaults = new DeviceDefaults();
        deviceDefaults.setTimeZoneManager(tzm);
        m_polycomPhoneDefaults = new PolycomPhoneDefaults(deviceDefaults, new SpeedDial());
    }

    // Negative numeric start/stop DST values would be in poor taste, even if they should
    // be ignored.
    public void assertNoStartStopNegativeValues() {
        Method methods[] = m_polycomPhoneDefaults.getClass().getMethods();
        for (Method method : methods) {
            System.out.println("name: " + method.getName());
            if (0 == method.getName().indexOf("getDst") && Integer.TYPE == method.getReturnType()) {
                try {
                    Object ret_value = method.invoke(m_polycomPhoneDefaults, (Object[]) null);
                    assertTrue("Negative return value: " + method.getName(), 0 <= (Integer) ret_value);
                } catch (Exception e) {
                    fail(e.getMessage());
                }
            }
        }
    }

    public void testNoStartStopNegativeValues() {
        setUpWithDeviceTimeZone(new DeviceTimeZone());

        // Even though these are being ignored, make sure they aren't negative.
        assertNoStartStopNegativeValues();
    }

    // http://en.wikipedia.org/wiki/Daylight_saving_time_around_the_world#North_America
    public void testCanadaEasternTimeZone() {
        setUpWithSpecifiedTimeZone("Canada/Eastern");

        // 5 hours behind GMT.
        assertEquals(-5 * DeviceTimeZone.SECONDS_PER_HOUR, m_polycomPhoneDefaults.getTimeZoneGmtOffset());

        // DST is observed.
        assertEquals(true, m_polycomPhoneDefaults.isDstEnabled());

        // Start: 2am on the second Sunday in March
        assertEquals(2, m_polycomPhoneDefaults.getDstStartTime()); // 02:00 LST
        assertEquals(false, m_polycomPhoneDefaults.isDstStartDayOfWeekLastInMonth()); // not last
        assertEquals(8, m_polycomPhoneDefaults.getDstStartDate()); // 2nd
        assertEquals(1, m_polycomPhoneDefaults.getDstStartDayOfWeek()); // Sunday
        assertEquals(3, m_polycomPhoneDefaults.getDstStartMonth()); // March

        // Stop: 2am on the first Sunday in November
        assertEquals(2, m_polycomPhoneDefaults.getDstStopTime()); // 02:00 LDT
        assertEquals(false, m_polycomPhoneDefaults.isDstStopDayOfWeekLastInMonth()); // not last
        assertEquals(1, m_polycomPhoneDefaults.getDstStopDate()); // 1st occurance
        assertEquals(1, m_polycomPhoneDefaults.getDstStopDayOfWeek()); // Sunday
        assertEquals(11, m_polycomPhoneDefaults.getDstStopMonth()); // November
    }

    // http://en.wikipedia.org/wiki/Daylight_saving_time_around_the_world#Saskatchewan
    public void testCanadaSaskatchewanTimeZone() {
        setUpWithSpecifiedTimeZone("Canada/Saskatchewan");

        // 6 hours behind GMT.
        assertEquals(-6 * DeviceTimeZone.SECONDS_PER_HOUR, m_polycomPhoneDefaults.getTimeZoneGmtOffset());

        // DST is not observed.
        assertEquals(false, m_polycomPhoneDefaults.isDstEnabled());

        // Even though these are being ignored, make sure they aren't negative.
        assertNoStartStopNegativeValues();
    }

    // http://en.wikipedia.org/wiki/European_Summer_Time
    public void testEuropeWarsawTimeZone() {
        setUpWithSpecifiedTimeZone("Europe/Warsaw");

        // 1 hour ahead of GMT.
        assertEquals(1 * DeviceTimeZone.SECONDS_PER_HOUR, m_polycomPhoneDefaults.getTimeZoneGmtOffset());

        // DST is observed.
        assertEquals(true, m_polycomPhoneDefaults.isDstEnabled());

        // Start: 1am on the last Sunday in March
        assertEquals(2, m_polycomPhoneDefaults.getDstStartTime()); // 01:00 GMT == 02:00 LST
        assertEquals(true, m_polycomPhoneDefaults.isDstStartDayOfWeekLastInMonth()); // last
                                                                                     // occurance
        assertEquals(1, m_polycomPhoneDefaults.getDstStartDayOfWeek(), 1); // Sunday
        assertEquals(3, m_polycomPhoneDefaults.getDstStartMonth(), 3); // March

        // Stop: 1am on the last Sunday in October
        assertEquals(3, m_polycomPhoneDefaults.getDstStopTime()); // 01:00 GMT == 03:00 LDT
        assertEquals(true, m_polycomPhoneDefaults.isDstStopDayOfWeekLastInMonth()); // last
                                                                                    // occurance
        assertEquals(1, m_polycomPhoneDefaults.getDstStopDayOfWeek()); // Sunday
        assertEquals(10, m_polycomPhoneDefaults.getDstStopMonth()); // October

        // Even though start/stop date are being ignored, make sure they aren't negative.
        testNoStartStopNegativeValues();
    }

}
