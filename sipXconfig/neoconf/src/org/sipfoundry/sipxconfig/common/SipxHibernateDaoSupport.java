/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.common;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;
import java.util.List;

import org.apache.commons.lang.ArrayUtils;
import org.hibernate.Criteria;
import org.hibernate.Session;
import org.hibernate.criterion.Order;
import org.hibernate.criterion.Projections;
import org.hibernate.criterion.Restrictions;
import org.hibernate.engine.SessionImplementor;
import org.hibernate.metadata.ClassMetadata;
import org.hibernate.persister.entity.EntityPersister;
import org.sipfoundry.sipxconfig.setting.BeanWithSettings;
import org.sipfoundry.sipxconfig.setting.Storage;
import org.sipfoundry.sipxconfig.setting.ValueStorage;
import org.springframework.orm.hibernate3.HibernateCallback;
import org.springframework.orm.hibernate3.HibernateTemplate;
import org.springframework.orm.hibernate3.support.HibernateDaoSupport;

public class SipxHibernateDaoSupport<T> extends HibernateDaoSupport {

    /**
     * Check if a bean exists w/o loading it or trying to load it and getting a
     * dataintegrityexcepiton
     */
    public boolean isBeanAvailable(Class c, Serializable id) {
        ClassMetadata classMetadata = getHibernateTemplate().getSessionFactory()
                .getClassMetadata(c);
        String name = classMetadata.getEntityName();
        List results = getHibernateTemplate().findByNamedParam(
                "select 1 from " + name + " where id = :id", "id", id);
        return !results.isEmpty();
    }

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
    public Object duplicateBean(BeanWithId bean, String queryName) {
        BeanWithId copy = bean.duplicate();

        if (bean instanceof NamedObject) {
            // Give the new bean a unique name by prepending "copyOf" to the source
            // bean's name until we get a name that hasn't been used yet.
            HibernateTemplate template = getHibernateTemplate();
            NamedObject namedCopy = (NamedObject) copy;
            namedCopy.setName(((NamedObject) bean).getName());
            do {
                namedCopy.setName("CopyOf" + namedCopy.getName());
            } while (DaoUtils.checkDuplicatesByNamedQuery(template, copy, queryName, namedCopy
                    .getName(), null));
        }

        return copy;
    }

    public List loadBeansByPage(Class beanClass, Integer groupId, int firstRow, int pageSize,
            String[] orderBy, boolean orderAscending) {
        Criteria c = getByGroupCriteria(beanClass, groupId);
        c.setFirstResult(firstRow);
        c.setMaxResults(pageSize);
        for (int i = 0; i < orderBy.length; i++) {
            Order order = orderAscending ? Order.asc(orderBy[i]) : Order.desc(orderBy[i]);
            c.addOrder(order);
        }
        List users = c.list();
        return users;
    }

    /**
     * Return the count of beans of type beanClass in the specified group. If groupId is null,
     * then don't filter by group, just count all the beans.
     */
    public int getBeansInGroupCount(Class beanClass, Integer groupId) {
        Criteria crit = getByGroupCriteria(beanClass, groupId);
        crit.setProjection(Projections.rowCount());
        List results = crit.list();
        if (results.size() > 1) {
            throw new RuntimeException("Querying for bean count returned multiple results!");
        }
        Integer count = (Integer) results.get(0);
        return count.intValue();
    }

    /**
     * Create and return a Criteria object for filtering beans by group membership. The class
     * passed in should extend BeanWithGroups. If groupId is null, then don't filter by group.
     */
    public Criteria getByGroupCriteria(Class klass, Integer groupId) {
        Criteria crit = getSession().createCriteria(klass);
        if (groupId != null) {
            crit.createCriteria("groups", "g");
            crit.add(Restrictions.eq("g.id", groupId));
        }
        return crit;
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

    static class GetOriginalValueCallback implements HibernateCallback {
        private PrimaryKeySource m_object;
        private String m_propertyName;

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
                throw new IllegalArgumentException("Property '" + m_propertyName
                        + "' not found on object '" + m_object.getClass() + "'");
            }

            Serializable id = (Serializable) m_object.getPrimaryKey();
            Object[] props = ep.getDatabaseSnapshot(id, si);
            return props[propIndex];
        }
    }
}
