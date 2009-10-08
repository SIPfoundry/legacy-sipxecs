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

import java.lang.reflect.Array;
import java.lang.reflect.InvocationTargetException;
import java.util.Collection;
import java.util.Iterator;
import java.util.List;

import org.apache.commons.beanutils.BeanUtils;
import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.collections.Transformer;
import org.apache.commons.collections.functors.ChainedTransformer;
import org.hibernate.Criteria;
import org.hibernate.Session;
import org.hibernate.criterion.Criterion;
import org.hibernate.criterion.Projections;
import org.hibernate.criterion.Restrictions;
import org.sipfoundry.sipxconfig.common.BeanWithId.IdToBean;
import org.sipfoundry.sipxconfig.common.event.DaoEventPublisher;
import org.sipfoundry.sipxconfig.setting.BeanWithGroups;
import org.sipfoundry.sipxconfig.setting.Group;
import org.springframework.orm.hibernate3.HibernateCallback;
import org.springframework.orm.hibernate3.HibernateTemplate;

/**
 * Utilities for Hibernate DAOs
 */
public final class DaoUtils {
    public static final int PAGE_SIZE = 250;
    private static final String ID_PROPERTY_NAME = "id";

    private DaoUtils() {
        // Utility class - do not instantiate
    }

    /**
     * Return true if query returns objects other than obj. Used to check for duplicates. The
     * query returns the ID strings of all objects for which the specified property has the
     * specified value. If exception is non-null, then throw the exception instead of returning
     * true.
     *
     * @param hibernate spring hibernate template
     * @param beanClass klass of the entity bean, it's usually one of the base classes of the obj
     *        and not obj.getClass()
     * @param bean object to be checked
     * @param propName name of the property to be checked
     * @param exception exception to throw if query returns other object than passed in the query
     */
    public static boolean checkDuplicates(HibernateTemplate hibernate, Class< ? extends BeanWithId> beanClass,
            BeanWithId bean, String propName, UserException exception) {
        Object propValue = getProperty_(bean, propName);
        if (propValue == null) {
            return false;
        }
        final Criterion expression = Restrictions.eq(propName, propValue);
        final Class klass = beanClass;
        HibernateCallback callback = new HibernateCallback() {
            public Object doInHibernate(Session session) {
                Criteria criteria = session.createCriteria(klass).add(expression).setProjection(
                        Projections.property(ID_PROPERTY_NAME));
                return criteria.list();
            }
        };
        List objs = hibernate.executeFind(callback);
        return checkDuplicates(bean, objs, exception);
    }

    /**
     * Return true if query returns objects other than obj. Used to check for duplicates. If
     * exception is non-null, then throw the exception instead of returning true.
     *
     * @param hibernate spring hibernate template
     * @param obj object to be checked
     * @param queryName name of the query to be executed (define in *.hbm.xml file)
     * @param value parameter for the query
     * @param exception exception to throw if query returns other object than passed in the query
     */
    public static boolean checkDuplicatesByNamedQuery(HibernateTemplate hibernate, BeanWithId obj, String queryName,
            Object value, UserException exception) {
        if (value == null) {
            return false;
        }

        List objs = hibernate.findByNamedQueryAndNamedParam(queryName, "value", value);
        return checkDuplicates(obj, objs, exception);
    }

    /**
     * Return true if list contains objects other than obj. Used to check for duplicates. If
     * exception is non-null, then throw the exception instead of returning true.
     *
     * @param obj object to be checked
     * @param objs results for query
     * @param exception exception to throw if query returns other object than passed in the query
     */
    public static boolean checkDuplicates(BeanWithId obj, Collection objs, UserException exception) {
        // no match
        if (objs.size() == 0) {
            return false; // there are no duplicates
        }

        // detect 1 match, itself
        if (!obj.isNew() && objs.size() == 1) {
            Integer found = (Integer) objs.iterator().next();
            if (found.equals(obj.getId())) {
                return false; // there are no duplicates
            }
        }

        // there are duplicates
        if (exception != null) {
            throw exception;
        }
        return true;
    }

    /**
     * Catch database corruption errors where more than one record exists. In general fields
     * should have unique indexes set up to protect against this. This method is created as a safe
     * check only, there have been not been any experiences of corrupt data to date.
     *
     * @param c
     * @param query
     *
     * @return first item from the collection
     * @throws IllegalStateException if more than one item in collection.
     */
    public static <T> T requireOneOrZero(Collection<T> c, String query) {
        if (c.size() > 1) {
            // DatabaseCorruptionException ?
            // TODO: move error string construction to new UnexpectedQueryResult(?) class, enable
            // localization
            StringBuffer error = new StringBuffer().append("read ").append(c.size()).append(
                    " and expected zero or one. query=").append(query);
            throw new IllegalStateException(error.toString());
        }
        Iterator<T> i = c.iterator();

        return (i.hasNext() ? c.iterator().next() : null);
    }

    // Put an underscore at the end of the method name to suppress a bogus
    // warning from Checkstyle about this method being unused.
    private static Object getProperty_(Object obj, String propName) {
        Object propValue = null;
        try {
            propValue = BeanUtils.getProperty(obj, propName);
        } catch (IllegalAccessException e) {
            throw new RuntimeException(e);
        } catch (InvocationTargetException e) {
            throw new RuntimeException(e);
        } catch (NoSuchMethodException e) {
            throw new RuntimeException(e);
        }
        return propValue;
    }

    /**
     * Returns the collection of loaded hibernate beans
     *
     * @param hibernate hibernate template
     * @param klass klass of the objects to be loaded
     * @param ids collection of object ids
     * @return newly created collection of objects loaded by hibernate
     */
    public static Collection loadBeanByIds(HibernateTemplate hibernate, Class klass, Collection ids) {
        IdToBean idToBean = new IdToBean(hibernate, klass);
        return CollectionUtils.collect(ids, idToBean);
    }

    /**
     * Returns array of beans loaded thru DataObjectSource
     */
    public static <T> T[] loadBeansArrayByIds(DataObjectSource source, Class<T> beanClass, Collection ids) {
        T[] beans = (T[]) Array.newInstance(beanClass, ids.size());
        Iterator idsIterator = ids.iterator();
        for (int i = 0; idsIterator.hasNext(); i++) {
            Integer id = (Integer) idsIterator.next();
            beans[i] = (T) source.load(beanClass, id);
        }

        return beans;
    }

    /**
     * Performs operation on all the bean in the list. List of the bean is passed as list of ids,
     * beans are loaded by hibernate before operation starts.
     *
     * After operation is performed all the beans are saved.
     */
    public static void doForAllBeanIds(HibernateTemplate hibernate, DaoEventPublisher eventPublisher,
            Transformer beanTransformer, Class klass, Collection ids) {
        IdToBean idToBean = new IdToBean(hibernate, klass);
        Transformer transformer = ChainedTransformer.getInstance(idToBean, beanTransformer);
        Collection beans = CollectionUtils.collect(ids, transformer);
        for (Object item : beans) {
            eventPublisher.publishSave(item);
        }
        hibernate.saveOrUpdateAll(beans);
    }

    public static void addToGroup(HibernateTemplate hibernate, DaoEventPublisher eventPublisher, Integer groupId,
            Class klass, Collection ids) {
        Group group = (Group) hibernate.load(Group.class, groupId);
        Transformer addTag = new BeanWithGroups.AddTag(group);
        doForAllBeanIds(hibernate, eventPublisher, addTag, klass, ids);
    }

    public static void removeFromGroup(HibernateTemplate hibernate, DaoEventPublisher eventPublisher,
            Integer groupId, Class klass, Collection ids) {
        Group group = (Group) hibernate.load(Group.class, groupId);
        Transformer addTag = new BeanWithGroups.RemoveTag(group);
        doForAllBeanIds(hibernate, eventPublisher, addTag, klass, ids);
    }

    /**
     * Executes the given closure for all users.
     *
     * Whenever something needs to be done for all users this method (rather than loadUsers should
     * be used) since it does not load all users in the memory.
     *
     * @param closure the closure to perform
     */
    public static void forAllUsersDo(CoreContext coreContext, Closure<User> closure) {
        int userIndex = 0;
        int size = 0;
        do {
            userIndex += size;
            List<User> users = coreContext.loadUsersByPage(userIndex, PAGE_SIZE);
            for (User user : users) {
                closure.execute(user);
            }
            size = users.size();
        } while (size == PAGE_SIZE);
    }
}
