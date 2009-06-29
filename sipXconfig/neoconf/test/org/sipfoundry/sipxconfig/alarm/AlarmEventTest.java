/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.alarm;

import java.text.DateFormat;
import java.text.SimpleDateFormat;

import org.sipfoundry.sipxconfig.admin.alarm.Alarm;

import org.sipfoundry.sipxconfig.admin.alarm.AlarmEvent;

import junit.framework.TestCase;

public class AlarmEventTest extends TestCase {
    private static final String LOG_ENTRY = "\"2009-06-22T13:39:56.406437Z\":4:ALARM:CRIT:sipx.example.org:sipXbridge::"
            + "SPX00013:\"The STUN Server 'stun01.sipphone.com' is dysfunctional.\"";

    private AlarmEvent m_alarmEvent;

    @Override
    protected void setUp() {
        m_alarmEvent = new AlarmEvent(LOG_ENTRY);
    }

    public void testParseAlarmLog() throws Exception {
        final Alarm alarm = m_alarmEvent.getAlarm();
        assertEquals("SPX00013", alarm.getCode());
        assertEquals("\"The STUN Server 'stun01.sipphone.com' is dysfunctional.\"", alarm.getDescription());
        assertEquals("CRIT", alarm.getSeverity());
        assertNotNull(m_alarmEvent.getDate());
        final DateFormat f = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
        assertEquals("2009-06-22 13:39:56", f.format(m_alarmEvent.getDate()));
    }
}
