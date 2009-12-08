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

import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.InputStream;
import java.io.Serializable;
import java.util.ArrayList;
import java.util.Collection;
import java.util.List;
import java.util.Set;

import static java.util.Collections.emptyList;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.admin.commserver.SipxReplicationContext;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.common.event.UserDeleteListener;
import org.springframework.beans.factory.annotation.Required;
import org.springframework.dao.support.DataAccessUtils;
import org.springframework.orm.hibernate3.HibernateTemplate;

public class AlarmServerManagerImpl extends SipxHibernateDaoSupport<AlarmGroup> implements AlarmServerManager {
    private static final Log LOG = LogFactory.getLog(AlarmServerManagerImpl.class);

    private static final String DEFAULT_HOST = "@localhost";
    private static final String PARAM_ALARM_GROUP_ID = "alarmGroupId";
    private static final String PARAM_ALARM_GROUP_NAME = "alarmGroupName";
    private static final String GROUP_NAME_DISABLED = "disabled";
    private static final String GROUP_NAME_DEFAULT = "default";

    private SipxReplicationContext m_replicationContext;
    private AlarmServerConfiguration m_alarmServerConfiguration;
    private AlarmConfiguration m_alarmsConfiguration;
    private AlarmGroupsConfiguration m_alarmGroupsConfiguration;
    private String m_sipxUser;
    private String m_logDirectory;
    private String m_configDirectory;
    private String m_alarmsStringsDirectory;
    private LocationsManager m_locationsManager;

    @Required
    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }

    @Required
    public void setReplicationContext(SipxReplicationContext replicationContext) {
        m_replicationContext = replicationContext;
    }

    @Required
    public void setAlarmServerConfiguration(AlarmServerConfiguration alarmServerConfiguration) {
        m_alarmServerConfiguration = alarmServerConfiguration;
    }

    @Required
    public void setAlarmsConfiguration(AlarmConfiguration alarmConfiguration) {
        m_alarmsConfiguration = alarmConfiguration;
    }

    @Required
    public void setAlarmGroupsConfiguration(AlarmGroupsConfiguration alarmGroupsConfiguration) {
        m_alarmGroupsConfiguration = alarmGroupsConfiguration;
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
            if (groupName.compareTo(alarm.getGroupName()) == 0) {
                // If this alarm's group name is being removed then use the disabled group name.
                alarm.setGroupName(GROUP_NAME_DISABLED);
            }
        }
    }

    public void removeAlarmGroups(Collection<Integer> groupsIds, List<Alarm> alarms) {
        for (Integer id : groupsIds) {
            // Remove matching group numbers from alarm before removing the alarm group.
            AlarmGroup group = (AlarmGroup) getHibernateTemplate().load(AlarmGroup.class, id);
            clearAlarmStorage(group.getName(), alarms);
            getHibernateTemplate().delete(group);
        }
    }

    @Required
    public void setLogDirectory(String logDirectory) {
        m_logDirectory = logDirectory;
    }

    @Required
    public void setConfigDirectory(String configDirectory) {
        m_configDirectory = configDirectory;
    }

    @Required
    public void setAlarmsStringsDirectory(String alarmsStringsDirectory) {
        m_alarmsStringsDirectory = alarmsStringsDirectory;
    }

    public void deployAlarmConfiguration(AlarmServer server, List<Alarm> alarms, List<AlarmGroup> groups) {
        // save the alarm codes
        saveAlarmCodes(alarms);
        // save alarm server configuration
        saveAlarmServer(server);
        // replicate new alarm server configuration
        replicateAlarmServer(m_replicationContext, null);
        // replicate new alarm types configuration
        replicateAlarmsConfiguration(alarms);
        // replicate new alarm groups configuration
        replicateAlarmGroupsConfiguration(groups);

        m_replicationContext.publishEvent(new AlarmServerActivatedEvent(this));
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
        server.setEmailNotificationEnabled(true);
        // set default email address (Group 1 contact email = SIPXPBXUSER@localhost)
        AlarmGroup group = createDefaultGroup();
        if (!isNameInUse(group)) {
            saveAlarmGroup(group);
        }
        return server;
    }

    private AlarmGroup createDefaultGroup() {
        AlarmGroup group = new AlarmGroup();
        List<String> addresses = new ArrayList<String>();
        addresses.add(m_sipxUser + DEFAULT_HOST);
        group.setEmailAddresses(addresses);
        group.setName(GROUP_NAME_DEFAULT);

        return group;
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

    public void replicateAlarmServer(SipxReplicationContext replicationContext, Location location) {
        AlarmServer alarmServer = getAlarmServer();
        m_alarmServerConfiguration.generate(alarmServer, m_logDirectory, getHost());

        if (location == null) {
            replicationContext.replicate(m_alarmServerConfiguration);
        } else {
            replicationContext.replicate(location, m_alarmServerConfiguration);
        }
    }

    private void replicateAlarmsConfiguration(List<Alarm> alarms) {
        m_alarmsConfiguration.generate(alarms);
        m_replicationContext.replicate(m_alarmsConfiguration);
    }

    private void replicateAlarmGroupsConfiguration(List<AlarmGroup> groups) {
        m_alarmGroupsConfiguration.generate(groups);
        m_replicationContext.replicate(m_alarmGroupsConfiguration);
    }

    private String getHost() {
        return m_locationsManager.getPrimaryLocation().getFqdn();
    }

    public List<AlarmGroup> getAlarmGroups() {
        return getHibernateTemplate().loadAll(AlarmGroup.class);
    }

    public AlarmGroup getAlarmGroupById(Integer alarmGroupId) {
        return (AlarmGroup) getHibernateTemplate().load(AlarmGroup.class, alarmGroupId);
    }

    public AlarmGroup getAlarmGroupByName(String alarmGroupName) {
        List<AlarmGroup> alarmGroups = getHibernateTemplate().loadAll(AlarmGroup.class);
        for (AlarmGroup alarmGroup : alarmGroups) {
            if (alarmGroup.getName() == alarmGroupName) {
                return alarmGroup;
            }
        }
        return null;
    }

    public void deleteAlarmGroupsById(Collection<Integer> groupsIds) {
        removeAll(AlarmGroup.class, groupsIds);
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

    public void clear() {
        removeAll(AlarmGroup.class);
    }

    public void saveAlarmGroup(AlarmGroup group) {
        if (group.isNew()) {
            // check if new object
            checkForDuplicateNames(group);
        } else {
            // on edit action - check if the group name for this group was modified
            // if the group name was changed then perform duplicate group name checking
            if (isNameChanged(group)) {
                checkForDuplicateNames(group);
            }
        }

        getHibernateTemplate().saveOrUpdate(group);
    }

    private void checkForDuplicateNames(AlarmGroup group) {
        if (isNameInUse(group)) {
            throw new UserException("&error.duplicateGroupNames", group.getName());
        }
    }

    private boolean isNameInUse(AlarmGroup group) {
        String groupName = group.getName();
        if (groupName.compareTo(GROUP_NAME_DISABLED) == 0) {
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
