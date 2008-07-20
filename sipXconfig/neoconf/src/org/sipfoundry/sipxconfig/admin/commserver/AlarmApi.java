/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 *
 */
package org.sipfoundry.sipxconfig.admin.commserver;

import java.util.Vector;

public interface AlarmApi {
    Boolean     raiseAlarm(String host, String alarmId, String alarmParam);
    Boolean     raiseAlarm(String host, String alarmId, Vector<String> alarmParams);
    Boolean     reloadAlarms(String host);
}
