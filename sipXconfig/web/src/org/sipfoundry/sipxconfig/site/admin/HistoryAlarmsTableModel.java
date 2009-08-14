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

import java.util.ArrayList;
import java.util.Date;
import java.util.Iterator;

import org.apache.tapestry.contrib.table.model.IBasicTableModel;
import org.apache.tapestry.contrib.table.model.ITableColumn;
import org.sipfoundry.sipxconfig.admin.alarm.AlarmEvent;
import org.sipfoundry.sipxconfig.common.AlarmContext;

public class HistoryAlarmsTableModel implements IBasicTableModel {

    private AlarmContext m_alarmContext;
    private String m_host;
    private Date m_startDate;
    private Date m_endDate;

    @Override
    public Iterator getCurrentPageRows(int first, int pageSize, ITableColumn sortColumn, boolean sortOrder) {
        if (!isValidConfig()) {
            return new ArrayList<AlarmEvent>().iterator();
        }

        return m_alarmContext.getAlarmEventsByPage(m_host, m_startDate, m_endDate, first, pageSize).iterator();
    }

    @Override
    public int getRowCount() {
        if (!isValidConfig()) {
            return 0;
        }
        return m_alarmContext.getAlarmEvents(m_host, m_startDate, m_endDate).size();
    }

    private boolean isValidConfig() {
        return m_host != null && m_startDate != null && m_endDate != null;
    }

    public void setHost(String host) {
        m_host = host;
    }

    public void setStartDate(Date startDate) {
        m_startDate = startDate;
    }

    public void setEndDate(Date endDate) {
        m_endDate = endDate;
    }

    public void setAlarmContext(AlarmContext alarmContext) {
        m_alarmContext = alarmContext;
    }
}
