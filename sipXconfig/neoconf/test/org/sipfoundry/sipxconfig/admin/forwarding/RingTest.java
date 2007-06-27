/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.admin.forwarding;

import java.util.Calendar;
import java.util.Date;
import java.util.TimeZone;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.admin.ScheduledDay;
import org.sipfoundry.sipxconfig.admin.dialplan.ForkQueueValue;
import org.sipfoundry.sipxconfig.admin.dialplan.attendant.WorkingTime;
import org.sipfoundry.sipxconfig.admin.dialplan.attendant.WorkingTime.WorkingHours;

/**
 * RingTest
 */
public class RingTest extends TestCase {

    public void testCalculateContact() {
        ForkQueueValue q = new ForkQueueValue(3);
        Ring ring = new Ring();
        ring.setNumber("555");
        ring.setExpiration(45);
        ring.setType(Ring.Type.IMMEDIATE);

        AbstractSchedule schedule = new Schedule();
        WorkingHours[] hours = new WorkingHours[1];
        WorkingTime wt = new WorkingTime();
        hours[0] = new WorkingHours();
        Calendar cal = Calendar.getInstance(TimeZone.getTimeZone("GMT"));
        cal.set(2006, Calendar.DECEMBER, 31, 10, 00);
        hours[0].setStart(cal.getTime());
        Integer startHour = Integer.valueOf(cal.get(Calendar.HOUR_OF_DAY));
        Integer startMinute = Integer.valueOf(cal.get(Calendar.MINUTE));
        cal.set(2006, Calendar.DECEMBER, 31, 11, 00);
        hours[0].setStop(cal.getTime());
        Integer stopHour = Integer.valueOf(cal.get(Calendar.HOUR_OF_DAY));
        Integer stopMinute = Integer.valueOf(cal.get(Calendar.MINUTE));
        hours[0].setEnabled(true);
        hours[0].setDay(ScheduledDay.WEDNESDAY);
        wt.setWorkingHours(hours);
        wt.setEnabled(true);
        schedule.setWorkingTime(wt);

        ring.setSchedule(schedule);

        int offset = TimeZone.getDefault().getOffset((new Date()).getTime()) / 60000;
        Integer minutesFromSunday = (hours[0].getDay().getDayOfWeek() - 1) * 24 * 60;
        Integer startWithTimezone = minutesFromSunday + startHour * 60 + startMinute - offset;
        Integer stopWithTimezone = minutesFromSunday + stopHour * 60 + stopMinute - offset;
        String expected = Integer.toHexString(startWithTimezone) + ":"
                + Integer.toHexString(stopWithTimezone);

        String contact = ring.calculateContact("sipfoundry.org", q, false);
        assertEquals("<sip:555@sipfoundry.org?expires=45>;q=1.0;sipx-ValidTime=" + expected,
                contact);
    }

    public void testCalculateAorContact() {
        ForkQueueValue q = new ForkQueueValue(3);
        Ring ring = new Ring();
        ring.setNumber("joe@example.com");
        ring.setExpiration(45);
        ring.setType(Ring.Type.IMMEDIATE);

        String contact = ring.calculateContact("shouldnt-be-used.com", q, false);
        assertEquals("<sip:joe@example.com?expires=45>;q=1.0;", contact);
    }
}
