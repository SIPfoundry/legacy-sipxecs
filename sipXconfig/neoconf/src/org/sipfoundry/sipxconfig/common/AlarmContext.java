/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.common;

public interface AlarmContext {
    enum Command {
        RAISE_ALARM, GET_ALARM_COUNT, RELOAD_ALARMS;
    }

    /**
     * Send an alarm to the Alarm Server with a list of runtime parameters.
     *
     * @param unique internal alarm id
     * @param list of runtime parameters
     */
    void raiseAlarm(String alarmId, String... alarmParams);

    /**
     * Send request to Alarm Server to reload alarms (after config or language change).
     */
    void reloadAlarms();
}
