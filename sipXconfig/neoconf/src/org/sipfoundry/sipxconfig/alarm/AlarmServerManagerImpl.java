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

import static java.util.Collections.emptyList;

import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.InputStream;
import java.io.Serializable;
import java.util.ArrayList;
import java.util.Collection;
import java.util.List;
import java.util.Set;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.common.event.UserDeleteListener;
import org.springframework.beans.factory.annotation.Required;
import org.springframework.dao.support.DataAccessUtils;
import org.springframework.orm.hibernate3.HibernateTemplate;
import org.springframework.util.CollectionUtils;

public class AlarmServerManagerImpl extends SipxHibernateDaoSupport<AlarmGroup> implements AlarmServerManager {
    private static final Log LOG = LogFactory.getLog(AlarmServerManagerImpl.class);

    private static final String DEFAULT_HOST = "@localhost";
    private static final String PARAM_ALARM_GROUP_ID = "alarmGroupId";
    private static final String PARAM_ALARM_GROUP_NAME = "alarmGroupName";
    private static final String GROUP_NAME_DISABLED = "disabled";
    private static final String GROUP_NAME_DEFAULT = "default";
    private static final String MIB_FILE_NAME = "SIPXECS-ALARM-NOTIFICATION-MIB.mib";

    private String m_sipxUser;
    private String m_logDirectory;
    private String m_configDirectory;
    private String m_alarmsStringsDirectory;
    private String m_mibsDirectory;
    private ConfigManager m_configManager;
    private LocationsManager m_locationsManager;

    @Required
    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }

    @Required
    public void setSipxUser(String sipxUser) {
        m_sipxUser = sipxUser;
    }

    public AlarmGroup loadAlarmGroup(Serializable id) {
        return (AlarmGroup) getHibernateTemplate().load(AlarmGroup.class, id);
    }

    private void clearAlarmStorage(String groupName, List<Alarm> alarms) {
        for (int i = 0; i < alarms.size(); i++) {
            Alarm alarm = alarms.get(i);
            if (groupName.equals(alarm.getGroupName())) {
                // If this alarm's group name is being removed then use the disabled group name.
                alarm.setGroupName(GROUP_NAME_DISABLED);
            }
        }
    }

    protected boolean isDefaultGroup(AlarmGroup group) {
        if (group.getName().equals(GROUP_NAME_DEFAULT)) {
            return true;
        }
        return false;
    }

    public boolean removeAlarmGroups(Collection<Integer> groupsIds, List<Alarm> alarms) {
        boolean affectDefaultGroup = false;
        for (Integer id : groupsIds) {
            AlarmGroup group = (AlarmGroup) getHibernateTemplate().load(AlarmGroup.class, id);
            // Don't delete the default group.
            if (!isDefaultGroup(group)) {
                // Remove matching group numbers from alarm before removing the alarm group.
                clearAlarmStorage(group.getName(), alarms);
                getHibernateTemplate().delete(group);
            } else {
                affectDefaultGroup = true;
            }
        }
        m_configManager.replicationRequired(Alarms.FEATURE);
        return affectDefaultGroup;

    }

    @Required
    public void setLogDirectory(String logDirectory) {
        m_logDirectory = logDirectory;
    }

    public String getLogDirectory() {
        return m_logDirectory;
    }

    @Required
    public void setConfigDirectory(String configDirectory) {
        m_configDirectory = configDirectory;
    }

    @Required
    public void setAlarmsStringsDirectory(String alarmsStringsDirectory) {
        m_alarmsStringsDirectory = alarmsStringsDirectory;
    }

    @Required
    public void setMibsDirectory(String mibsDirectory) {
        m_mibsDirectory = mibsDirectory;
    }

    /**
     * Creates the alarm server if it does not exist.
     * Checks if the 'default' alarm group has email contact(s).
     * Used ony on FirstRunTask.
     */
    public void deployAlarms() {
        getAlarmServer();
        //we don't need to replicate since it will be done anyway in initLocations()
        //a strange race condition will trigger some NPEs
        updateDefaultAlarmGroup();
    }

    public void deployAlarmConfiguration(AlarmServer server, List<Alarm> alarms) {
        saveAlarmCodes(alarms);
        saveAlarmServer(server);
    }

    public AlarmServer getAlarmServer() {
        List servers = getHibernateTemplate().loadAll(AlarmServer.class);
        AlarmServer server = (AlarmServer) DataAccessUtils.singleResult(servers);
        if (server == null) {
            server = newAlarmServer();
            saveAlarmServer(server);
        }

        return server;
    }

    private AlarmServer newAlarmServer() {
        AlarmServer server = new AlarmServer();
        // set email notification enabled by default
        server.setAlarmNotificationEnabled(true);
        return server;
    }

    private void updateDefaultAlarmGroup() {
        AlarmGroup defaultAlarmGroup = getAlarmGroupByName(GROUP_NAME_DEFAULT);
        List<String> addresses = defaultAlarmGroup.getEmailAddresses();
        if (CollectionUtils.isEmpty(addresses)) {
            addresses.add(m_sipxUser + DEFAULT_HOST);
            defaultAlarmGroup.setEmailAddresses(addresses);
            saveAlarmGroup(defaultAlarmGroup);
        }
    }

    private void saveAlarmServer(AlarmServer server) {
        HibernateTemplate template = getHibernateTemplate();
        template.saveOrUpdate(server);
        template.flush();
    }

    private void saveAlarmCodes(List<Alarm> alarmCodes) {
        HibernateTemplate template = getHibernateTemplate();
        Collection oldAlarms = template.loadAll(Alarm.class);
        template.deleteAll(oldAlarms);
        template.saveOrUpdateAll(new ArrayList(alarmCodes));
        template.flush();
    }

    public String getHost() {
        return m_locationsManager.getPrimaryLocation().getFqdn();
    }

    public List<AlarmGroup> getAlarmGroups() {
        List<AlarmGroup> groups = getHibernateTemplate().loadAll(AlarmGroup.class);

        return groups;
    }

    public AlarmGroup getAlarmGroupById(Integer alarmGroupId) {
        return (AlarmGroup) getHibernateTemplate().load(AlarmGroup.class, alarmGroupId);
    }

    public AlarmGroup getAlarmGroupByName(String alarmGroupName) {
        List<AlarmGroup> alarmGroups = getAlarmGroups();
        for (AlarmGroup alarmGroup : alarmGroups) {
            if (alarmGroup.getName().equals(alarmGroupName)) {
                return alarmGroup;
            }
        }

        return null;
    }

    public List<Alarm> getAlarmTypes() {
        try {
            InputStream isAlarmsConfig = new FileInputStream(m_configDirectory + "/alarms/sipXalarms-config.xml");
            InputStream isAlarmsString = new FileInputStream(m_alarmsStringsDirectory + "/sipXalarms-strings.xml");
            AlarmTypesParser parser = new AlarmTypesParser(getHibernateTemplate());
            return parser.getTypes(isAlarmsConfig, isAlarmsString);
        } catch (FileNotFoundException e) {
            LOG.error("Cannot find alarm definitions", e);
            return emptyList();
        }
    }

    @Override
    public void saveAlarmGroup(AlarmGroup group) {
        if (group.isNew()) {
            // check if new object
            checkForDuplicateNames(group);
            getHibernateTemplate().save(group);
        } else {
            // on edit action - check if the group name for this group was modified
            // if the group name was changed then perform duplicate group name checking
            if (isNameChanged(group)) {
                checkForDuplicateNames(group);
                // don't rename the default group
                AlarmGroup defaultAlarmGroup = getAlarmGroupByName(GROUP_NAME_DEFAULT);
                if (defaultAlarmGroup != null) {
                    if (defaultAlarmGroup.getId().equals(group.getId())
                            && !group.getName().equals(GROUP_NAME_DEFAULT)) {
                        throw new UserException("&msg.defalutAlarmGroupRename");
                    }
                }
            }
            getHibernateTemplate().merge(group);
        }
    }

    @Override
    public String getMibsDirectory() {
        return m_mibsDirectory;
    }

    @Override
    public String getAlarmNotificationMibFileName() {
        return MIB_FILE_NAME;
    }

    private void checkForDuplicateNames(AlarmGroup group) {
        if (isNameInUse(group)) {
            throw new UserException("&error.duplicateGroupNames", group.getName());
        }
    }

    private boolean isNameInUse(AlarmGroup group) {
        String groupName = group.getName();
        if (groupName.equals(GROUP_NAME_DISABLED)) {
            return true;
        }
        List count = getHibernateTemplate().findByNamedQueryAndNamedParam("anotherAlarmGroupWithSameName",
                new String[] {
                    PARAM_ALARM_GROUP_NAME
                }, new Object[] {
                    groupName
                });

        return DataAccessUtils.intResult(count) > 0;
    }

    private boolean isNameChanged(AlarmGroup group) {
        List count = getHibernateTemplate().findByNamedQueryAndNamedParam("countAlarmGroupWithSameName",
                new String[] {
                    PARAM_ALARM_GROUP_ID, PARAM_ALARM_GROUP_NAME
                }, new Object[] {
                    group.getId(), group.getName()
                });

        return DataAccessUtils.intResult(count) == 0;
    }

    public UserDeleteListener createUserDeleteListener() {
        return new OnUserDelete();
    }

    private class OnUserDelete extends UserDeleteListener {
        @Override
        protected void onUserDelete(User user) {
            List<AlarmGroup> groups = getAlarmGroups();
            for (AlarmGroup group : groups) {
                Set<User> users = group.getUsers();
                if (users.remove(user)) {
                    getHibernateTemplate().saveOrUpdate(group);
                }
            }
        }
    }
}
