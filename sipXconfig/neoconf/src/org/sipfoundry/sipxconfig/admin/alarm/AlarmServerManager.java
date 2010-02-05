/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.admin.alarm;

import java.io.Serializable;
import java.util.Collection;
import java.util.List;

public interface AlarmServerManager {
    AlarmServer getAlarmServer();

    List<AlarmGroup> getAlarmGroups();

    boolean removeAlarmGroups(Collection<Integer> groupsIds, List<Alarm> alarms);

    AlarmGroup loadAlarmGroup(Serializable id);

    AlarmGroup getAlarmGroupById(Integer alarmGroupId);

    AlarmGroup getAlarmGroupByName(String alarmGroupName);

    void saveAlarmGroup(AlarmGroup group);

    void deployAlarmConfiguration(AlarmServer alarmServer, List<Alarm> alarms);

    List<Alarm> getAlarmTypes();

    String getLogDirectory();

    String getHost();
}
