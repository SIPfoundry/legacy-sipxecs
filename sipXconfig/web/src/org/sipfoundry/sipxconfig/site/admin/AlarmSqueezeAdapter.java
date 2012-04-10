/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.admin;

import java.util.List;

import org.apache.tapestry.components.IPrimaryKeyConverter;
import org.sipfoundry.sipxconfig.alarm.Alarm;

public class AlarmSqueezeAdapter implements IPrimaryKeyConverter {
    private List<Alarm> m_alarms;

    public AlarmSqueezeAdapter(List<Alarm> alarms) {
        m_alarms = alarms;
    }

    public Object getPrimaryKey(Object value) {
        Alarm alarm = (Alarm) value;
        return alarm.getAlarmDefinition().getId();
    }

    public Object getValue(Object primaryKey) {
        for (Alarm alarm : m_alarms) {
            if (alarm.getAlarmDefinition().getId().equals(primaryKey)) {
                return alarm;
            }
        }
        return null;
    }
}
