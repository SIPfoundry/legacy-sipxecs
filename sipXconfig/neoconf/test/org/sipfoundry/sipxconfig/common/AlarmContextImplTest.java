/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */

package org.sipfoundry.sipxconfig.common;

import java.io.InputStream;
import java.util.Calendar;
import java.util.Date;
import java.util.List;

import org.sipfoundry.sipxconfig.admin.alarm.AlarmEvent;

import junit.framework.TestCase;

public class AlarmContextImplTest extends TestCase {

    public void testParseEventsStream() throws Exception {
        AlarmContextImpl impl = new AlarmContextImpl();
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

}
