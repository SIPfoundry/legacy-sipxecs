/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */

package org.sipfoundry.sipxconfig.admin.alarm;

import java.io.InputStream;
import java.util.Calendar;
import java.util.Date;
import java.util.List;

import junit.framework.TestCase;
import org.sipfoundry.sipxconfig.admin.alarm.AlarmEvent;
import org.sipfoundry.sipxconfig.admin.alarm.AlarmHistoryManagerImpl;

public class AlarmHistoryManagerImplTest extends TestCase {

    public void testParseEventsStream() throws Exception {
        AlarmHistoryManagerImpl impl = new AlarmHistoryManagerImpl();
        Calendar calendar = Calendar.getInstance();
        calendar.clear();
        calendar.set(2009, Calendar.JUNE, 20);
        Date startDate = calendar.getTime();
        calendar.add(Calendar.DAY_OF_MONTH, 3);
        Date endDate = calendar.getTime();
        InputStream stream = getClass().getResourceAsStream("alarm.test.log");
        List<AlarmEvent> events = impl.parseEventsStream(stream, startDate, endDate);
        assertEquals(1, events.size());
        AlarmEvent event = events.get(0);
        assertEquals("SPX00013", event.getAlarm().getCode());
    }

    public void testParseEventsStreamByPage() throws Exception {
        AlarmHistoryManagerImpl impl = new AlarmHistoryManagerImpl();
        Calendar calendar = Calendar.getInstance();
        calendar.clear();
        calendar.set(2009, Calendar.JUNE, 20);
        Date startDate = calendar.getTime();
        calendar.add(Calendar.DAY_OF_MONTH, 3);
        Date endDate = calendar.getTime();
        InputStream stream = getClass().getResourceAsStream("alarm.test.large.log");
        List<AlarmEvent> events = impl.parseEventsStreamByPage(stream, startDate, endDate, 0, 10);
        assertEquals(10, events.size());
        assertEquals("SPX00013", events.get(0).getAlarm().getCode());

        stream = getClass().getResourceAsStream("alarm.test.large.log");
        events = impl.parseEventsStreamByPage(stream, startDate, endDate, 10, 10);
        assertEquals(10, events.size());
        assertEquals("SPX00023", events.get(0).getAlarm().getCode());

        stream = getClass().getResourceAsStream("alarm.test.large.log");
        events = impl.parseEventsStreamByPage(stream, startDate, endDate, 20, 10);
        assertEquals(3, events.size());
        assertEquals("SPX00033", events.get(0).getAlarm().getCode());
    }
}
