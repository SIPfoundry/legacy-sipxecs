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

import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.List;

import org.apache.commons.collections.Predicate;
import org.hibernate.criterion.DetachedCriteria;
import org.hibernate.criterion.Order;
import org.hibernate.criterion.Projections;
import org.hibernate.criterion.Restrictions;
import org.sipfoundry.commons.util.ShortHash;
import org.sipfoundry.sipxconfig.admin.intercom.Intercom;
import org.sipfoundry.sipxconfig.admin.intercom.IntercomManager;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.DaoUtils;
import org.sipfoundry.sipxconfig.common.DataCollectionUtil;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.sipfoundry.sipxconfig.common.SpecialUser.SpecialUserType;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.common.event.DaoEventListener;
import org.sipfoundry.sipxconfig.common.event.DaoEventPublisher;
import org.sipfoundry.sipxconfig.device.DeviceDefaults;
import org.sipfoundry.sipxconfig.device.ProfileLocation;
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
import org.springframework.orm.hibernate3.HibernateTemplate;

import static org.apache.commons.collections.CollectionUtils.select;
import static org.apache.commons.lang.StringUtils.isEmpty;

/**
 * Context for entire sipXconfig framework. Holder for service layer bean factories.
 */
public class PhoneContextImpl extends SipxHibernateDaoSupport implements BeanFactoryAware, PhoneContext,
        ApplicationListener, DaoEventListener {

    private static final String QUERY_PHONE_ID_BY_SERIAL_NUMBER = "phoneIdsWithSerialNumber";

    private static final String USER_ID = "userId";

    private CoreContext m_coreContext;

    private SettingDao m_settingDao;

    private BeanFactory m_beanFactory;

    private String m_systemDirectory;

    private DeviceDefaults m_deviceDefaults;

    private IntercomManager m_intercomManager;

    private PhonebookManager m_phonebookManager;

    private SpeedDialManager m_speedDialManager;

    private DaoEventPublisher m_daoEventPublisher;

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

    @Required
    public void setDaoEventPublisher(DaoEventPublisher daoEventPublisher) {
        m_daoEventPublisher = daoEventPublisher;
    }

    /**
     * Callback that supplies the owning factory to a bean instance.
     */
    public void setBeanFactory(BeanFactory beanFactory) {
        m_beanFactory = beanFactory;
    }

    public void flush() {
        getHibernateTemplate().flush();
    }

    public void storePhone(Phone phone) {
        HibernateTemplate hibernate = getHibernateTemplate();
        String serialNumber = phone.getSerialNumber();
        if (!phone.getModel().isSerialNumberValid(serialNumber)) {
            throw new InvalidSerialNumberException(serialNumber, phone.getModel().getSerialNumberPattern());
        }
        DaoUtils.checkDuplicatesByNamedQuery(hibernate, phone, QUERY_PHONE_ID_BY_SERIAL_NUMBER, serialNumber,
                new DuplicateSerialNumberException(serialNumber));
        phone.setValueStorage(clearUnsavedValueStorage(phone.getValueStorage()));
        hibernate.saveOrUpdate(phone);
    }

    public void deletePhone(Phone phone) {
        ProfileLocation location = phone.getModel().getDefaultProfileLocation();
        phone.removeProfiles(location);
        phone.setValueStorage(clearUnsavedValueStorage(phone.getValueStorage()));
        for (Line line : phone.getLines()) {
            line.setValueStorage(clearUnsavedValueStorage(line.getValueStorage()));
        }
        getHibernateTemplate().delete(phone);
    }

    public void storeLine(Line line) {
        line.setValueStorage(clearUnsavedValueStorage(line.getValueStorage()));
        getHibernateTemplate().saveOrUpdate(line);
    }

    public void deleteLine(Line line) {
        line.setValueStorage(clearUnsavedValueStorage(line.getValueStorage()));
        getHibernateTemplate().delete(line);
    }

    public Line loadLine(Integer id) {
        Line line = (Line) getHibernateTemplate().load(Line.class, id);
        return line;
    }

    public int getPhonesCount() {
        return getPhonesInGroupCount(null);
    }

    public int getPhonesInGroupCount(Integer groupId) {
        return getBeansInGroupCount(Phone.class, groupId);
    }

    public List<Phone> loadPhonesByPage(Integer groupId, Integer branchId, int firstRow, int pageSize,
            String[] orderBy, boolean orderAscending) {
        return loadBeansByPage(Phone.class, groupId, branchId, firstRow, pageSize, orderBy, orderAscending);
    }

    public List<Phone> loadPhones() {
        return getHibernateTemplate().loadAll(Phone.class);
    }

    public List<Integer> getAllPhoneIds() {
        return getHibernateTemplate().findByNamedQuery("phoneIds");
    }

    public Phone loadPhone(Integer id) {
        Phone phone = (Phone) getHibernateTemplate().load(Phone.class, id);

        return phone;
    }

    public Integer getPhoneIdBySerialNumber(String serialNumber) {
        List objs = getHibernateTemplate().findByNamedQueryAndNamedParam(QUERY_PHONE_ID_BY_SERIAL_NUMBER, "value",
                serialNumber);
        return (Integer) DaoUtils.requireOneOrZero(objs, QUERY_PHONE_ID_BY_SERIAL_NUMBER);
    }

    public Phone newPhone(PhoneModel model) {
        Phone phone = (Phone) m_beanFactory.getBean(model.getBeanId());
        phone.setModel(model);
        return phone;
    }

    public List<Group> getGroups() {
        return m_settingDao.getGroups(GROUP_RESOURCE_ID);
    }

    public Group getGroupByName(String phoneGroupName, boolean createIfNotFound) {
        if (createIfNotFound) {
            return m_settingDao.getGroupCreateIfNotFound(GROUP_RESOURCE_ID, phoneGroupName);
        }
        return m_settingDao.getGroupByName(GROUP_RESOURCE_ID, phoneGroupName);
    }

    /** unittesting only */
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

    public String getSystemDirectory() {
        return m_systemDirectory;
    }

    public void setSystemDirectory(String systemDirectory) {
        m_systemDirectory = systemDirectory;
    }

    private static class DuplicateSerialNumberException extends UserException {
        private static final String ERROR = "A phone with serial number: {0} already exists.";

        public DuplicateSerialNumberException(String serialNumber) {
            super(ERROR, serialNumber);
        }
    }

    private static class InvalidSerialNumberException extends UserException {
        private static final String ERROR = "The serial number \"{0}\" does not fulfill the required pattern {1}.";

        public InvalidSerialNumberException(String serialNumber, String pattern) {
            super(ERROR, serialNumber, pattern);
        }
    }

    public void onApplicationEvent(ApplicationEvent event_) {
        // no init tasks defined yet
    }

    public DeviceDefaults getPhoneDefaults() {
        return m_deviceDefaults;
    }

    public void setPhoneDefaults(DeviceDefaults deviceDefaults) {
        m_deviceDefaults = deviceDefaults;
    }

    public Collection getPhonesByGroupId(Integer groupId) {
        Collection users = getHibernateTemplate().findByNamedQueryAndNamedParam("phonesByGroupId", "groupId",
                groupId);
        return users;
    }

    public void onDelete(Object entity) {
        Class c = entity.getClass();
        if (Group.class.equals(c)) {
            Group group = (Group) entity;
            getHibernateTemplate().update(group);
            if (Phone.GROUP_RESOURCE_ID.equals(group.getResource())) {
                Collection<Phone> phones = getPhonesByGroupId(group.getId());
                for (Phone phone : phones) {
                    Object[] ids = new Object[] {
                        group.getId()
                    };
                    DataCollectionUtil.removeByPrimaryKey(phone.getGroups(), ids);
                    storePhone(phone);
                }
            }
        } else if (User.class.equals(c)) {
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

    public void onSave(Object entity_) {
    }

    public Collection<Phone> getPhonesByUserId(Integer userId) {
        return getHibernateTemplate().findByNamedQueryAndNamedParam("phonesByUserId", USER_ID, userId);
    }

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

    public void addToGroup(Integer groupId, Collection<Integer> ids) {
        DaoUtils.addToGroup(getHibernateTemplate(), m_daoEventPublisher, groupId, Phone.class, ids);
    }

    public void removeFromGroup(Integer groupId, Collection<Integer> ids) {
        DaoUtils.removeFromGroup(getHibernateTemplate(), m_daoEventPublisher, groupId, Phone.class, ids);
    }

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

    public Intercom getIntercomForPhone(Phone phone) {
        return m_intercomManager.getIntercomForPhone(phone);
    }

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

    public SpeedDial getSpeedDial(Phone phone) {
        User user = phone.getPrimaryUser();
        if (user != null) {
            return m_speedDialManager.getSpeedDialForUserId(user.getId(), false);
        }
        return null;
    }

    public Collection<Integer> getPhoneIdsByUserGroupId(int groupId) {
        Group group = m_coreContext.getGroupById(groupId);
        Collection<User> users = m_coreContext.getGroupMembers(group);
        Collection<Integer> ids = new ArrayList<Integer>();

        for (User user : users) {
            Collection<Phone> phones = getPhonesByUserId(user.getId());
            ids.addAll(DataCollectionUtil.extractPrimaryKeys(phones));
        }
        return ids;
    }

    public User createSpecialPhoneProvisionUser(String serialNumber) {
        User user = m_coreContext.getSpecialUser(SpecialUserType.PHONE_PROVISION);

        user.setFirstName("ID:");
        user.setLastName(ShortHash.get(serialNumber));

        return user;
    }

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
}
