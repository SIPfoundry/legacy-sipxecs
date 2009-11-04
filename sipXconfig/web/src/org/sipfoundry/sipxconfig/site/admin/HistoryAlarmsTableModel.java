/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.site.admin;

import java.util.Collections;
import java.util.Iterator;
import java.util.List;

import org.apache.tapestry.contrib.table.model.IBasicTableModel;
import org.apache.tapestry.contrib.table.model.ITableColumn;
import org.sipfoundry.sipxconfig.admin.alarm.AlarmEvent;
import org.sipfoundry.sipxconfig.admin.alarm.AlarmHistoryManager;

public class HistoryAlarmsTableModel implements IBasicTableModel {

    private AlarmHistoryManager m_alarmHistoryManager;
    private List<AlarmEvent> m_alarmEvents;

    @Override
    public Iterator getCurrentPageRows(int first, int pageSize, ITableColumn sortColumn, boolean sortOrder) {
        if (m_alarmEvents == null) {
            return Collections.emptyList().iterator();
        }

        List<AlarmEvent> alarms = m_alarmHistoryManager.getAlarmEventsByPage(m_alarmEvents, first, pageSize);
        return alarms.iterator();
    }

    @Override
    public int getRowCount() {
        if (m_alarmEvents == null) {
            return 0;
        }
        return m_alarmEvents.size();
    }

    public void setAlarmHistoryManager(AlarmHistoryManager alarmHistoryManager) {
        m_alarmHistoryManager = alarmHistoryManager;
    }

    public void setAlarmEvents(List<AlarmEvent> alarmEvents) {
        m_alarmEvents = alarmEvents;
    }

}
