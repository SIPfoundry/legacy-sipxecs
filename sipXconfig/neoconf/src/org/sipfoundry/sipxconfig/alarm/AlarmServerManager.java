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

import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.snmp.SnmpManager;

public interface AlarmServerManager {
    public static final AlarmDefinition CPU_THRESHOLD_EXCEEDED = new AlarmDefinition("CPU_THRESHOLD_EXCEEDED", 90);
    public static final AlarmDefinition CPU_THRESHOLD_RECOVERED = new AlarmDefinition("CPU_THRESHOLD_RECOVERED", 40);
    public static final AlarmDefinition DISK_USAGE_THRESHOLD_EXCEEDED = new AlarmDefinition(
            "DISK_USAGE_THRESHOLD_EXCEEDED", 95);
    public static final AlarmDefinition DISK_USAGE_THRESHOLD_RECOVERED = new AlarmDefinition(
            "DISK_USAGE_THRESHOLD_RECOVERED", 80);

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

    Collection<String> getActiveMonitorConfiguration(SnmpManager snmpManager, Collection<Alarm> alarms,
            Location location);

    /**
     * key is alarm definition id
     * @return
     */
    Map<String, AlarmDefinition> getAlarmDefinitions();

    FeatureManager getFeatureManager();
}
