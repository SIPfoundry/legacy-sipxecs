/*
 * 
 * 
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.admin;

import java.util.Vector;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.admin.commserver.SipxAlarmContext;

/** Raise alarm to be handled as configured by Alarm Server */
public class Alarm {

    private static final Log LOG = LogFactory.getLog(Alarm.class);

    private static SipxAlarmContext s_alarmContext;

    public void setAlarmContext(SipxAlarmContext alarmContext) {
        s_alarmContext = alarmContext;
    }

    /**
     * Send an alarm to the Alarm Server with a single runtime parameter.
     * 
     * @param unique internal alarm id
     * @param single runtime parameter
     */
    public static void raiseAlarm(String alarmId, String alarmParam) {
        Vector<String> alarmParams = new Vector<String>();
        alarmParams.add(alarmParam);
        raiseAlarm(alarmId, alarmParams);
    }

    /**
     * Send an alarm to the Alarm Server with a list of runtime parameters.
     * 
     * @param unique internal alarm id
     * @param list of runtime parameters
     */
    public static void raiseAlarm(String alarmId, Vector<String> alarmParams) {
        try {
            if (s_alarmContext == null) {
                LOG.debug("raiseAlarm: s_alarmContext has not been set");
                LOG.error("could not raise alarm " + alarmId + " " + alarmParams);
            } else {
                s_alarmContext.raiseAlarm(alarmId, alarmParams);
            }
        } catch (Exception e) {
            LOG.error("caught exception in alarm handling; ignoring so we don't make it worse",
                    e);
        }
    }

    /**
     * Send request to Alarm Server to reload alarms (after config or language change).
     */
    public static void reloadAlarms() {
        try {
            if (s_alarmContext == null) {
                LOG.debug("reloadAlarms: s_alarmContext has not been set");
                LOG.error("could not request reloadAlarms");
            } else {
                s_alarmContext.reloadAlarms();
            }
        } catch (Exception e) {
            LOG.error("caught exception in reloadAlarms; ignoring so we don't make it worse", 
                    e);
        }
    }
}
