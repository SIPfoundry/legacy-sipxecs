/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.admin;

import java.io.InputStream;
import java.util.Calendar;
import java.util.Date;
import java.util.Iterator;
import java.util.List;

import org.junit.Before;
import org.junit.Test;
import org.sipfoundry.sipxconfig.admin.alarm.AlarmEvent;
import org.sipfoundry.sipxconfig.common.AlarmContextImpl;

import static org.junit.Assert.assertEquals;

public class HistoryAlarmsTableModelTest {

    private HistoryAlarmsTableModel m_out;
    private DummyAlarmContext m_alarmContext;

    @Before
    public void setUp() {
        m_out = new HistoryAlarmsTableModel();
        m_alarmContext = new DummyAlarmContext();
        m_out.setAlarmContext(m_alarmContext);

        Calendar calendar = Calendar.getInstance();
        calendar.clear();
        calendar.set(2009, Calendar.JUNE, 20);
        Date startDate = calendar.getTime();
        calendar.add(Calendar.DAY_OF_MONTH, 3);
        Date endDate = calendar.getTime();

        m_out.setHost("localhost");
        m_out.setStartDate(startDate);
        m_out.setEndDate(endDate);
    }

    @Test
    public void testGetCurrentPageRows() {
        countPageRows(10, m_out.getCurrentPageRows(0, 10, null, false));
        countPageRows(10, m_out.getCurrentPageRows(10, 10, null, false));
        countPageRows(3, m_out.getCurrentPageRows(20, 10, null, false));
    }

    @Test
    public void testGetRowCount() {
        assertEquals(23, m_out.getRowCount());
    }

    private void countPageRows(int expectedCount, Iterator<AlarmEvent> alarmIter) {
        int count = 0;
        while (alarmIter.hasNext()) {
            alarmIter.next();
            count++;
        }
        assertEquals(expectedCount, count);
    }

    private static class DummyAlarmContext extends AlarmContextImpl {
        @Override
        public List<AlarmEvent> getAlarmEventsByPage(String host, Date startDate, Date endDate, int first, int pageSize) {
            InputStream stream = getClass().getResourceAsStream("alarm.test.large.log");
            try {
                return parseEventsStreamByPage(stream, startDate, endDate, first, pageSize);
            } catch (Exception e) {
                throw new RuntimeException(e);
            }
        }

        @Override
        public List<AlarmEvent> getAlarmEvents(String host, Date startDate, Date endDate) {
            InputStream stream = getClass().getResourceAsStream("alarm.test.large.log");
            try {
                return parseEventsStream(stream, startDate, endDate);
            } catch (Exception e) {
                throw new RuntimeException(e);
            }
        }
    }

}
