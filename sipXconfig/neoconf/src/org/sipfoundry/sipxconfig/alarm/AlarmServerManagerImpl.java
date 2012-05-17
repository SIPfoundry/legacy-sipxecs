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
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.common.event.DaoEventListener;
import org.sipfoundry.sipxconfig.commserver.LocationsManager;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.beans.factory.ListableBeanFactory;
import org.springframework.beans.factory.annotation.Required;
import org.springframework.dao.support.DataAccessUtils;
import org.springframework.jdbc.core.BatchPreparedStatementSetter;
import org.springframework.jdbc.core.JdbcTemplate;
import org.springframework.jdbc.core.RowCallbackHandler;
import org.springframework.orm.hibernate3.HibernateTemplate;
import org.springframework.util.CollectionUtils;

public class AlarmServerManagerImpl extends SipxHibernateDaoSupport<AlarmGroup> implements AlarmServerManager,
        DaoEventListener, BeanFactoryAware {
    private static final Log LOG = LogFactory.getLog(AlarmServerManagerImpl.class);
    private static final String DEFAULT_HOST = "@localhost";
    private static final String PARAM_ALARM_GROUP_ID = "alarmGroupId";
    private static final String PARAM_ALARM_GROUP_NAME = "alarmGroupName";
    private static final String GROUP_NAME_DISABLED = "disabled";
    private static final String GROUP_NAME_DEFAULT = "default";
    private static final String MIB_FILE_NAME = "SIPXECS-ALARM-NOTIFICATION-MIB.mib";
    private ListableBeanFactory m_beanFactory;
    private String m_sipxUser;
    private String m_logDirectory;
    private String m_configDirectory;
    private String m_alarmsStringsDirectory;
    private String m_mibsDirectory;
    private LocationsManager m_locationsManager;
    private Set<AlarmProvider> m_providers;
    private Map<String, AlarmDefinition> m_definitions;
    private JdbcTemplate m_jdbcTemplate;

    public Map<String, AlarmDefinition> getAlarmDefinitions() {
        Map<String, AlarmDefinition> defs = new HashMap<String, AlarmDefinition>();
        for (AlarmProvider p : getAlarmProviders()) {
            Collection<AlarmDefinition> avail = p.getAvailableAlarms(this);
            if (avail != null) {
                for (AlarmDefinition d : avail) {
                    defs.put(d.getId(), d);
                }
            }
        }
        return defs;
    }

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
                getDaoEventPublisher().publishDelete(group);
            } else {
                affectDefaultGroup = true;
            }
        }
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

    @Override
    public void saveAlarmServer(AlarmServer server) {
        HibernateTemplate template = getHibernateTemplate();
        template.saveOrUpdate(server);
        template.flush();
    }

    @Override
    public void saveAlarms(final List<Alarm> alarms) {
        m_jdbcTemplate.execute("delete from alarm_code");
        BatchPreparedStatementSetter save = new BatchPreparedStatementSetter() {

            @Override
            public int getBatchSize() {
                return alarms.size();
            }

            @Override
            public void setValues(PreparedStatement ps, int i) throws SQLException {
                Alarm a = alarms.get(i);
                ps.setString(1, a.getAlarmDefinition().getId());
                ps.setString(2, a.getGroupName());
                ps.setInt(3, a.getMinThreshold());
            }
        };
        String sql = "insert into alarm_code (alarm_code_id, email_group, min_threshold) values (?, ?, ?)";
        m_jdbcTemplate.batchUpdate(sql, save);
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

    public List<Alarm> getAlarms() {
        final List<Alarm> alarms = new ArrayList<Alarm>();
        final Map<String, AlarmDefinition> defs = new HashMap<String, AlarmDefinition>(getAlarmDefinitions());
        RowCallbackHandler rows = new RowCallbackHandler() {

            @Override
            public void processRow(ResultSet rs) throws SQLException {
                String id = rs.getString("alarm_code_id");
                AlarmDefinition d = defs.get(id);
                if (d == null) {
                    return;
                }
                Alarm a = new Alarm(d);
                defs.remove(id);
                a.setGroupName(rs.getString("email_group"));
                a.setMinThreshold(rs.getInt("min_threshold"));
                alarms.add(a);
            }
        };

        m_jdbcTemplate.query("select * from alarm_code", rows);
        for (AlarmDefinition def : defs.values()) {
            Alarm a = new Alarm(def);
            alarms.add(a);
        }
        return alarms;
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

    @Override
    public void onDelete(Object entity) {
        if (!(entity instanceof User)) {
            return;
        }

        List<AlarmGroup> groups = getAlarmGroups();
        for (AlarmGroup group : groups) {
            Set<User> users = group.getUsers();
            if (users.remove((User) entity)) {
                getHibernateTemplate().saveOrUpdate(group);
            }
        }
    }

    @Override
    public void onSave(Object entity) {
    }

    @Override
    public void setBeanFactory(BeanFactory beanFactory) {
        m_beanFactory = (ListableBeanFactory) beanFactory;
    }

    Set<AlarmProvider> getAlarmProviders() {
        if (m_providers == null) {
            Map<String, AlarmProvider> beanMap = m_beanFactory.getBeansOfType(
                    AlarmProvider.class, false, false);
            m_providers = new HashSet<AlarmProvider>(beanMap.values());
        }

        return m_providers;
    }

    public void setJdbcTemplate(JdbcTemplate jdbcTemplate) {
        m_jdbcTemplate = jdbcTemplate;
    }

    @Override
    public List<AlarmTrapReceiver> getAlarmTrapReceivers() {
        return getHibernateTemplate().loadAll(AlarmTrapReceiver.class);
    }

    @Override
    public void saveAlarmTrapReceiver(AlarmTrapReceiver r) {
        getHibernateTemplate().saveOrUpdate(r);
    }

    @Override
    public void deleteAlarmTrapReceiver(AlarmTrapReceiver r) {
        getHibernateTemplate().delete(r);
    }

    @Override
    public void saveAlarmTrapReceivers(List<AlarmTrapReceiver> receivers) {
        getHibernateTemplate().saveOrUpdateAll(receivers);
    }
}
