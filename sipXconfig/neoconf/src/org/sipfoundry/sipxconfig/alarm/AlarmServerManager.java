/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.alarm;

import java.io.Serializable;
import java.util.Collection;
import java.util.List;
import java.util.Map;

public interface AlarmServerManager {
    AlarmServer getAlarmServer();

    List<AlarmGroup> getAlarmGroups();

    List<AlarmTrapReceiver> getAlarmTrapReceivers();

    void saveAlarmTrapReceivers(List<AlarmTrapReceiver> receivers);

    void saveAlarmTrapReceiver(AlarmTrapReceiver r);

    void deleteAlarmTrapReceiver(AlarmTrapReceiver r);

    boolean removeAlarmGroups(Collection<Integer> groupsIds, List<Alarm> alarms);

    AlarmGroup loadAlarmGroup(Serializable id);

    AlarmGroup getAlarmGroupById(Integer alarmGroupId);

    AlarmGroup getAlarmGroupByName(String alarmGroupName);

    void saveAlarmGroup(AlarmGroup group);

    void saveAlarmServer(AlarmServer server);

    void saveAlarms(List<Alarm> alarms);

    List<Alarm> getAlarms();

    String getLogDirectory();

    String getHost();

    String getMibsDirectory();

    String getAlarmNotificationMibFileName();

    /**
     * key is alarm definition id
     * @return
     */
    Map<String, AlarmDefinition> getAlarmDefinitions();
}
