/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.phone;

import static org.apache.commons.collections.CollectionUtils.select;
import static org.apache.commons.lang.StringUtils.isEmpty;

import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.LinkedList;
import java.util.List;

import org.apache.commons.collections.Predicate;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.hibernate.criterion.DetachedCriteria;
import org.hibernate.criterion.Order;
import org.hibernate.criterion.Projections;
import org.hibernate.criterion.Restrictions;
import org.sipfoundry.commons.util.ShortHash;
import org.sipfoundry.sipxconfig.alarm.AlarmDefinition;
import org.sipfoundry.sipxconfig.alarm.AlarmProvider;
import org.sipfoundry.sipxconfig.alarm.AlarmServerManager;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.DaoUtils;
import org.sipfoundry.sipxconfig.common.DataCollectionUtil;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.sipfoundry.sipxconfig.common.SpecialUser.SpecialUserType;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.device.DeviceDefaults;
import org.sipfoundry.sipxconfig.device.ProfileLocation;
import org.sipfoundry.sipxconfig.intercom.Intercom;
import org.sipfoundry.sipxconfig.intercom.IntercomManager;
import org.sipfoundry.sipxconfig.phonebook.GooglePhonebookEntry;
import org.sipfoundry.sipxconfig.phonebook.Phonebook;
import org.sipfoundry.sipxconfig.phonebook.PhonebookEntry;
import org.sipfoundry.sipxconfig.phonebook.PhonebookManager;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.SettingDao;
import org.sipfoundry.sipxconfig.speeddial.SpeedDial;
import org.sipfoundry.sipxconfig.speeddial.SpeedDialManager;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.beans.factory.annotation.Required;
import org.springframework.context.ApplicationEvent;
import org.springframework.context.ApplicationListener;
import org.springframework.dao.support.DataAccessUtils;
import org.springframework.jdbc.core.JdbcTemplate;
import org.springframework.jdbc.core.RowCallbackHandler;
import org.springframework.orm.hibernate3.HibernateTemplate;

/**
 * Context for entire sipXconfig framework. Holder for service layer bean factories.
 */
public class PhoneContextImpl extends SipxHibernateDaoSupport implements BeanFactoryAware, PhoneContext,
        ApplicationListener, AlarmProvider {
    private static final Log LOG = LogFactory.getLog(PhoneContextImpl.class);
    private static final String QUERY_PHONE_ID_BY_SERIAL_NUMBER = "phoneIdsWithSerialNumber";
    private static final String QUERY_PHONE_BY_SERIAL_NUMBER = "phoneWithSerialNumber";
    private static final String ALARM_PHONE_ADDED = "ALARM_PHONE_ADDED Phone with serial %s was added to the system.";
    private static final String ALARM_PHONE_CHANGED = "ALARM_PHONE_CHANGED Phone with id: %d, serial: %s was changed.";
    private static final String ALARM_PHONE_DELETED = "ALARM_PHONE_DELETED Phone with id: %d, serial: %s was deleted.";

    private static final String USER_ID = "userId";
    private static final String VALUE = "value";
    private static final String SQL_SELECT_GROUP = "select p.phone_id,p.serial_number "
            + "from phone p join phone_group pg on pg.phone_id = p.phone_id " + "where pg.group_id=%d";
    private CoreContext m_coreContext;

    private SettingDao m_settingDao;

    private BeanFactory m_beanFactory;

    private String m_systemDirectory;

    private DeviceDefaults m_deviceDefaults;

    private IntercomManager m_intercomManager;

    private PhonebookManager m_phonebookManager;

    private SpeedDialManager m_speedDialManager;

    private JdbcTemplate m_jdbcTemplate;

    @Required
    public void setPhonebookManager(PhonebookManager phonebookManager) {
        m_phonebookManager = phonebookManager;
    }

    @Required
    public void setSettingDao(SettingDao settingDao) {
        m_settingDao = settingDao;
    }

    @Required
    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    @Required
    public void setIntercomManager(IntercomManager intercomManager) {
        m_intercomManager = intercomManager;
    }

    @Required
    public void setSpeedDialManager(SpeedDialManager speedDialManager) {
        m_speedDialManager = speedDialManager;
    }

    public void setConfigJdbcTemplate(JdbcTemplate template) {
        m_jdbcTemplate = template;
    }

    /**
     * Callback that supplies the owning factory to a bean instance.
     */
    @Override
    public void setBeanFactory(BeanFactory beanFactory) {
        m_beanFactory = beanFactory;
    }

    @Override
    public void flush() {
        getHibernateTemplate().flush();
    }

    @Override
    public void storePhone(Phone phone) {
        boolean isNew;
        HibernateTemplate hibernate = getHibernateTemplate();
        String serialNumber = phone.getSerialNumber();
        if (!phone.getModel().isSerialNumberValid(serialNumber)) {
            throw new InvalidSerialNumberException(serialNumber, phone.getModel().getSerialNumberPattern());
        }
        DaoUtils.checkDuplicatesByNamedQuery(hibernate, phone, QUERY_PHONE_ID_BY_SERIAL_NUMBER, serialNumber,
                new DuplicateSerialNumberException(serialNumber));
        phone.setValueStorage(clearUnsavedValueStorage(phone.getValueStorage()));
        isNew = phone.isNew();
        hibernate.saveOrUpdate(phone);
        if (isNew) {
            LOG.error(String.format(ALARM_PHONE_ADDED, phone.getSerialNumber()));
        }
        getDaoEventPublisher().publishSave(phone);
    }

    @Override
    public void deletePhone(Phone phone) {
        ProfileLocation location = phone.getModel().getDefaultProfileLocation();
        phone.removeProfiles(location);
        phone.setValueStorage(clearUnsavedValueStorage(phone.getValueStorage()));
        for (Line line : phone.getLines()) {
            line.setValueStorage(clearUnsavedValueStorage(line.getValueStorage()));
        }
        getHibernateTemplate().delete(phone);
        LOG.error(String.format(ALARM_PHONE_DELETED, phone.getId(), phone.getSerialNumber()));
    }

    @Override
    public void storeLine(Line line) {
        line.setValueStorage(clearUnsavedValueStorage(line.getValueStorage()));
        getHibernateTemplate().saveOrUpdate(line);
        getDaoEventPublisher().publishSave(line);
    }

    @Override
    public void deleteLine(Line line) {
        line.setValueStorage(clearUnsavedValueStorage(line.getValueStorage()));
        getHibernateTemplate().delete(line);
    }

    @Override
    public Line loadLine(Integer id) {
        Line line = getHibernateTemplate().load(Line.class, id);
        return line;
    }

    @Override
    public int getPhonesCount() {
        return getPhonesInGroupCount(null);
    }

    @Override
    public int getPhonesInGroupCount(Integer groupId) {
        return getBeansInGroupCount(Phone.class, groupId);
    }

    @Override
    public List<Phone> loadPhonesByPage(Integer groupId, int firstRow, int pageSize, String[] orderBy,
            boolean orderAscending) {
        return loadBeansByPage(Phone.class, groupId, firstRow, pageSize, orderBy, orderAscending);
    }

    @Override
    public List<Phone> loadPhonesByPage(int firstRow, int pageSize) {
        return loadBeansByPage(Phone.class, firstRow, pageSize);
    }

    @Override
    public List<Phone> loadPhones() {
        return getHibernateTemplate().loadAll(Phone.class);
    }

    @Override
    public List<Integer> getAllPhoneIds() {
        return getHibernateTemplate().findByNamedQuery("phoneIds");
    }

    @Override
    public Phone loadPhone(Integer id) {
        Phone phone = getHibernateTemplate().load(Phone.class, id);

        return phone;
    }

    @Override
    public Integer getPhoneIdBySerialNumber(String serialNumber) {
        List objs = getHibernateTemplate().findByNamedQueryAndNamedParam(QUERY_PHONE_ID_BY_SERIAL_NUMBER, VALUE,
                serialNumber);
        return (Integer) DaoUtils.requireOneOrZero(objs, QUERY_PHONE_ID_BY_SERIAL_NUMBER);
    }

    @Override
    public Phone getPhoneBySerialNumber(String serialNumber) {
        List<Phone> objs = getHibernateTemplate().findByNamedQueryAndNamedParam(QUERY_PHONE_BY_SERIAL_NUMBER, VALUE,
                serialNumber);
        return DaoUtils.requireOneOrZero(objs, QUERY_PHONE_BY_SERIAL_NUMBER);
    }

    @Override
    public Phone newPhone(PhoneModel model) {
        Phone phone = (Phone) m_beanFactory.getBean(model.getBeanId());
        phone.setModel(model);
        return phone;
    }

    @Override
    public List<Group> getGroups() {
        return m_settingDao.getGroups(GROUP_RESOURCE_ID);
    }

    @Override
    public void saveGroup(Group group) {
        m_settingDao.saveGroup(group);
    }

    @Override
    public boolean deleteGroups(Collection<Integer> groupIds) {
        return m_settingDao.deleteGroups(groupIds);
    }

    @Override
    public Group getGroupByName(String phoneGroupName, boolean createIfNotFound) {
        if (createIfNotFound) {
            return m_settingDao.getGroupCreateIfNotFound(GROUP_RESOURCE_ID, phoneGroupName);
        }
        return m_settingDao.getGroupByName(GROUP_RESOURCE_ID, phoneGroupName);
    }

    /** unittesting only */
    @Override
    public void clear() {
        // ordered bottom-up, e.g. traverse foreign keys so as to
        // not leave hanging references. DB will reject otherwise
        deleteAll("from Phone");
        deleteAll("from Group where resource = 'phone'");
    }

    private void deleteAll(String query) {
        Collection c = getHibernateTemplate().find(query);
        getHibernateTemplate().deleteAll(c);
    }

    @Override
    public String getSystemDirectory() {
        return m_systemDirectory;
    }

    public void setSystemDirectory(String systemDirectory) {
        m_systemDirectory = systemDirectory;
    }

    private static class DuplicateSerialNumberException extends UserException {
        public DuplicateSerialNumberException(String serialNumber) {
            super("&error.duplicateSerialNumberException", serialNumber);
        }
    }

    private static class InvalidSerialNumberException extends UserException {
        private static final String ERROR = "&error.invalidSerialNumberException";

        public InvalidSerialNumberException(String serialNumber, String pattern) {
            super(ERROR, serialNumber, pattern);
        }
    }

    @Override
    public void onApplicationEvent(ApplicationEvent event_) {
        // no init tasks defined yet
    }

    @Override
    public DeviceDefaults getPhoneDefaults() {
        return m_deviceDefaults;
    }

    public void setPhoneDefaults(DeviceDefaults deviceDefaults) {
        m_deviceDefaults = deviceDefaults;
    }

    @Override
    public Collection<Phone> getPhonesByGroupId(Integer groupId) {
        Collection<Phone> phones = getHibernateTemplate().findByNamedQueryAndNamedParam("phonesByGroupId",
                "groupId", groupId);
        return phones;
    }

    @Override
    public void onDelete(Object entity) {
        Class c = entity.getClass();
        if (User.class.equals(c)) {
            User user = (User) entity;
            Collection<Phone> phones = getPhonesByUserId(user.getId());
            for (Phone phone : phones) {
                List<Integer> ids = new ArrayList<Integer>();
                Collection<Line> lines = phone.getLines();
                for (Line line : lines) {
                    User lineUser = line.getUser();
                    if (lineUser != null && lineUser.getId().equals(user.getId())) {
                        ids.add(line.getId());
                    }
                }
                DataCollectionUtil.removeByPrimaryKey(lines, ids.toArray());
                storePhone(phone);
            }
        }
    }

    private void raiseGroupAlarm(int groupId) {
        m_jdbcTemplate.query(String.format(SQL_SELECT_GROUP, groupId), new RowCallbackHandler() {

            @Override
            public void processRow(ResultSet rs) throws SQLException {
                LOG.error(String.format(ALARM_PHONE_CHANGED, rs.getInt("phone_id"), rs.getString("serial_number")));
            }
        });
    }

    @Override
    public void onSave(Object entity) {
        if (entity instanceof Phone) {
            Phone phone = (Phone) entity;
            LOG.error(String.format(ALARM_PHONE_CHANGED, phone.getId(), phone.getSerialNumber()));
        } else if (entity instanceof Group) {
            Group g = (Group) entity;
            if (g.getResource().equals(Phone.GROUP_RESOURCE_ID)) {
                raiseGroupAlarm(g.getId());
            }
        }
    }

    @Override
    public Collection<Phone> getPhonesByUserId(Integer userId) {
        return getHibernateTemplate().findByNamedQueryAndNamedParam("phonesByUserId", USER_ID, userId);
    }

    @Override
    public Collection<Phone> getPhonesByUserIdAndPhoneModel(Integer userId, String modelId) {
        String[] paramsNames = {
            USER_ID, "modelId"
        };
        Object[] paramsValues = {
            userId, modelId
        };
        Collection<Phone> phones = getHibernateTemplate().findByNamedQueryAndNamedParam(
                "phonesByUserIdAndPhoneModel", paramsNames, paramsValues);
        return phones;
    }

    @Override
    public void addToGroup(Integer groupId, Collection<Integer> ids) {
        DaoUtils.addToGroup(getHibernateTemplate(), getDaoEventPublisher(), groupId, Phone.class, ids);
    }

    @Override
    public void removeFromGroup(Integer groupId, Collection<Integer> ids) {
        DaoUtils.removeFromGroup(getHibernateTemplate(), getDaoEventPublisher(), groupId, Phone.class, ids);
    }

    @Override
    public void addUsersToPhone(Integer phoneId, Collection<Integer> ids) {
        Phone phone = loadPhone(phoneId);
        for (Integer userId : ids) {
            User user = m_coreContext.loadUser(userId);
            Line line = phone.createLine();
            line.setUser(user);
            phone.addLine(line);
        }
        storePhone(phone);
    }

    @Override
    public Intercom getIntercomForPhone(Phone phone) {
        return m_intercomManager.getIntercomForPhone(phone);
    }

    @Override
    public Collection<PhonebookEntry> getPhonebookEntries(Phone phone) {
        User user = phone.getPrimaryUser();
        if (user != null) {
            Collection<Phonebook> books = filterPhonebooks(m_phonebookManager.getAllPhonebooksByUser(user));
            return filterPhonebookEntries(m_phonebookManager.getEntries(books, user));
        }
        return Collections.emptyList();
    }

    private Collection<PhonebookEntry> filterPhonebookEntries(Collection<PhonebookEntry> entries) {
        Collection entriesToRemove = select(entries, new InvalidGoogleEntrySearchPredicate());
        entries.removeAll(entriesToRemove);
        return entries;
    }

    private Collection<Phonebook> filterPhonebooks(Collection<Phonebook> books) {
        Collection<Phonebook> filteredPhonebooks = new ArrayList<Phonebook>();
        for (Phonebook book : books) {
            if (book.getShowOnPhone()) {
                filteredPhonebooks.add(book);
            }
        }
        return filteredPhonebooks;
    }

    @Override
    public SpeedDial getSpeedDial(Phone phone) {
        User user = phone.getPrimaryUser();
        if (user != null && !user.isNew()) {
            return m_speedDialManager.getSpeedDialForUserId(user.getId(), false);
        }
        return null;
    }

    @Override
    public Collection<Integer> getPhoneIdsByUserGroupId(int groupId) {
        final List<Integer> userIds = new LinkedList<Integer>();
        m_jdbcTemplate.query("SELECT u.user_id from users u inner join user_group g "
                + "on u.user_id = g.user_id WHERE group_id=" + groupId + " AND u.user_type='C' "
                + "ORDER BY u.user_id;", new RowCallbackHandler() {
                    @Override
                    public void processRow(ResultSet rs) throws SQLException {
                        userIds.add(rs.getInt("user_id"));
                    }
                });
        Collection<Integer> ids = new ArrayList<Integer>();

        for (Integer userId : userIds) {
            Collection<Phone> phones = getPhonesByUserId(userId);
            ids.addAll(DataCollectionUtil.extractPrimaryKeys(phones));
        }
        return ids;
    }

    @Override
    public User createSpecialPhoneProvisionUser(String serialNumber) {
        User user = m_coreContext.getSpecialUser(SpecialUserType.PHONE_PROVISION);
        user.setFirstName("ID:");
        user.setLastName(ShortHash.get(serialNumber));

        return user;
    }

    @Override
    public List<Phone> loadPhonesWithNoLinesByPage(int firstRow, int pageSize, String[] orderBy,
            boolean orderAscending) {
        DetachedCriteria c = DetachedCriteria.forClass(Phone.class);
        addByNoLinesCriteria(c);
        if (orderBy != null) {
            for (String o : orderBy) {
                Order order = orderAscending ? Order.asc(o) : Order.desc(o);
                c.addOrder(order);
            }
        }
        return getHibernateTemplate().findByCriteria(c, firstRow, pageSize);
    }

    @Override
    public int getPhonesWithNoLinesCount() {
        DetachedCriteria crit = DetachedCriteria.forClass(Phone.class);
        addByNoLinesCriteria(crit);
        crit.setProjection(Projections.rowCount());
        List results = getHibernateTemplate().findByCriteria(crit);
        return (Integer) DataAccessUtils.requiredSingleResult(results);
    }

    public static void addByNoLinesCriteria(DetachedCriteria crit) {
        crit.add(Restrictions.isEmpty("lines"));
    }

    static class InvalidGoogleEntrySearchPredicate implements Predicate {

        @Override
        public boolean evaluate(Object phonebookEntry) {
            if (phonebookEntry instanceof GooglePhonebookEntry) {
                GooglePhonebookEntry entry = (GooglePhonebookEntry) phonebookEntry;
                return ((isEmpty(entry.getFirstName()) && isEmpty(entry.getLastName())) || isEmpty(entry.getNumber()));
            }
            return false;
        }

    }

    @Override
    public Collection<AlarmDefinition> getAvailableAlarms(AlarmServerManager manager) {
        Collection<AlarmDefinition> alarms = Arrays.asList(new AlarmDefinition[] {
            PhoneContext.ALARM_PHONE_ADDED, PhoneContext.ALARM_PHONE_DELETED, PhoneContext.ALARM_PHONE_CHANGED
        });
        return alarms;
    }
}
