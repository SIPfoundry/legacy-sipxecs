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

import org.apache.tapestry.components.IPrimaryKeyConverter;
import org.sipfoundry.sipxconfig.admin.alarm.Alarm;

public class AlarmSqueezeAdapter implements IPrimaryKeyConverter {
    public Object getPrimaryKey(Object value) {
        Alarm alarm = (Alarm) value;
        return alarm.getAlarmId();
    }

    public Object getValue(Object primaryKey) {
        Alarm alarm = new Alarm();
        alarm.setAlarmId(primaryKey.toString());
        return alarm;
    }
}
