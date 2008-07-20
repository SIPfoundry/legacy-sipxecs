/*
 * 
 * 
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.admin.commserver;

import java.util.Vector;


public interface SipxAlarmContext {
    public static enum Command {
        RAISE_ALARM, GET_ALARM_COUNT, RELOAD_ALARMS;
    }

    public void raiseAlarm(String alarmId, Vector<String> alarmParams);
    
    public void reloadAlarms();
}
