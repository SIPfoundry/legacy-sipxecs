/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.common;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;
import java.util.List;

import org.apache.commons.lang.ArrayUtils;
import org.hibernate.Session;
import org.hibernate.criterion.DetachedCriteria;
import org.hibernate.criterion.Order;
import org.hibernate.criterion.Projections;
import org.hibernate.criterion.Restrictions;
import org.hibernate.engine.SessionImplementor;
import org.hibernate.persister.entity.EntityPersister;
import org.sipfoundry.sipxconfig.setting.BeanWithSettings;
import org.sipfoundry.sipxconfig.setting.Storage;
import org.sipfoundry.sipxconfig.setting.ValueStorage;
import org.springframework.dao.support.DataAccessUtils;
import org.springframework.orm.hibernate3.HibernateCallback;
import org.springframework.orm.hibernate3.HibernateTemplate;
import org.springframework.orm.hibernate3.support.HibernateDaoSupport;

public class SipxHibernateDaoSupport<T> extends HibernateDaoSupport implements DataObjectSource<T> {

    public T load(Class<T> c, Serializable id) {
        return (T) getHibernateTemplate().load(c, id);
    }

    protected void saveBeanWithSettings(BeanWithSettings bean) {
        Storage origStorage = bean.getValueStorage();
        Storage cleanStorage = clearUnsavedValueStorage(origStorage);
        bean.setValueStorage(cleanStorage);
        getHibernateTemplate().saveOrUpdate(bean);
    }

    protected void deleteBeanWithSettings(BeanWithSettings bean) {
        // avoid hibernate errors about new object references when calling delete on parent object
        bean.setValueStorage(clearUnsavedValueStorage(bean.getValueStorage()));
        getHibernateTemplate().delete(bean);
    }

    /**
     * Duplicate the bean and return the duplicate. If the bean is a NamedObject, then give the
     * duplicate a new, unique name. The queryName identifies a named query that returns the IDs
     * of all objects with a given name. (Return IDs rather than objects to avoid the overhead of
     * loading all the objects.) Use the query to ensure that the new name is unique.
     *
     * @param bean bean to duplicate
     * @param queryName name of the query to be executed (define in *.hbm.xml file)
     */
    public BeanWithId duplicateBean(BeanWithId bean, String queryName) {
        BeanWithId copy = bean.duplicate();

        if (bean instanceof NamedObject) {
            // Give the new bean a unique name by prepending "copyOf" to the source
            // bean's name until we get a name that hasn't been used yet.
            HibernateTemplate template = getHibernateTemplate();
            NamedObject namedCopy = (NamedObject) copy;
            namedCopy.setName(((NamedObject) bean).getName());
            do {
                namedCopy.setName("CopyOf" + namedCopy.getName());
            } while (DaoUtils.checkDuplicatesByNamedQuery(template, copy, queryName, namedCopy.getName(), null));
        }

        return copy;
    }

    public List<T> loadBeansByPage(Class beanClass, Integer groupId, int firstRow, int pageSize,
            String[] orderBy, boolean orderAscending) {
        return loadBeansByPage(beanClass, groupId, null, firstRow, pageSize, orderBy, orderAscending);
    }

    public List<T> loadBeansByPage(Class beanClass, Integer groupId, Integer branchId, int firstRow, int pageSize,
            String[] orderBy, boolean orderAscending) {
        DetachedCriteria c = DetachedCriteria.forClass(beanClass);
        addByGroupCriteria(c, groupId);
        addByBranchCriteria(c, branchId);
        if (orderBy != null) {
            for (String o : orderBy) {
                Order order = orderAscending ? Order.asc(o) : Order.desc(o);
                c.addOrder(order);
            }
        }
        return getHibernateTemplate().findByCriteria(c, firstRow, pageSize);
    }

    public List<T> loadBeansByPage(Class beanClass, int firstRow, int pageSize) {
        String[] orderBy = new String[] {
            "id"
        };
        return loadBeansByPage(beanClass, null, null, firstRow, pageSize, orderBy, true);
    }

    /**
     * Return the count of beans of type beanClass in the specified group. If groupId is null,
     * then don't filter by group, just count all the beans.
     */
    public int getBeansInGroupCount(Class beanClass, Integer groupId) {
        DetachedCriteria crit = DetachedCriteria.forClass(beanClass);
        addByGroupCriteria(crit, groupId);
        crit.setProjection(Projections.rowCount());
        List results = getHibernateTemplate().findByCriteria(crit);
        return (Integer) DataAccessUtils.requiredSingleResult(results);
    }

    protected void removeAll(Class<T> klass, Collection ids) {
        HibernateTemplate template = getHibernateTemplate();
        Collection entities = new ArrayList(ids.size());
        for (Iterator i = ids.iterator(); i.hasNext();) {
            Integer id = (Integer) i.next();
            Object entity = template.load(klass, id);
            entities.add(entity);
        }
        template.deleteAll(entities);
        // HACK: this is to fix XCF-1732, it should not be needed but FLASH_AUTO strategy does not
        // work here
        template.flush();
    }

    protected void removeAll(Class<T> klass) {
        HibernateTemplate template = getHibernateTemplate();
        List entities = template.loadAll(klass);
        template.deleteAll(entities);
    }

    protected Storage clearUnsavedValueStorage(Storage storage) {
        // requirement, otherwise you wouldn't be calling this function
        ValueStorage vs = (ValueStorage) storage;

        // If no settings don't bother saving anything.
        return vs != null && vs.isNew() && vs.size() == 0 ? null : vs;
    }

    /**
     * Returns the original value of an object before it was modified by application. Represent
     * the original value from the database.
     */
    protected Object getOriginalValue(PrimaryKeySource obj, String propertyName) {
        HibernateCallback callback = new GetOriginalValueCallback(obj, propertyName);
        Object originalValue = getHibernateTemplate().execute(callback, true);
        return originalValue;
    }

    /**
     * Update a Criteria object for filtering beans by group membership. If groupId is null, then
     * don't filter by group.
     */
    public static void addByGroupCriteria(DetachedCriteria crit, Integer groupId) {
        if (groupId != null) {
            crit.createCriteria("groups", "g").add(Restrictions.eq("g.id", groupId));
        }
    }

    /**
     * Update a Criteria object for filtering beans by branch membership. If brnachId is null,
     * then don't filter by branch.
     */
    public static void addByBranchCriteria(DetachedCriteria crit, Integer branchId) {
        if (branchId != null) {
            crit.createCriteria("branch", "b").add(Restrictions.eq("b.id", branchId));
        }
    }

    static class GetOriginalValueCallback implements HibernateCallback {
        private final PrimaryKeySource m_object;
        private final String m_propertyName;

        GetOriginalValueCallback(PrimaryKeySource object, String propertyName) {
            m_object = object;
            m_propertyName = propertyName;
        }

        public Object doInHibernate(Session session) {
            SessionImplementor si = (SessionImplementor) session;
            EntityPersister ep = si.getEntityPersister(null, m_object);

            String[] propNames = ep.getPropertyNames();
            int propIndex = ArrayUtils.indexOf(propNames, m_propertyName);
            if (propIndex < 0) {
                throw new IllegalArgumentException("Property '" + m_propertyName + "' not found on object '"
                        + m_object.getClass() + "'");
            }

            Serializable id = (Serializable) m_object.getPrimaryKey();
            Object[] props = ep.getDatabaseSnapshot(id, si);
            return props[propIndex];
        }
    }
}
