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
import java.lang.reflect.InvocationTargetException;

import org.apache.commons.beanutils.BeanUtils;
import org.apache.commons.collections.Transformer;
import org.springframework.orm.hibernate3.HibernateTemplate;

/**
 * BeanWithId - simplify implementation of the model layer
 *
 * Hibernate advises against using object IDs in equals and hashCode methods, as we do below. See
 * http://www.hibernate.org/109.html . However, we disagree. It's true that using the ID means
 * that an unsaved object (with ID = -1) doesn't have a unique identity and that can cause
 * problems. However, if you use "business keys" like, for example, the object name, in the equals
 * and hashcode methods, then the object identity changes if you change the values of those keys,
 * which is also bad. We feel that the unsaved object problems are easier to deal with.
 */
public class BeanWithId implements PrimaryKeySource, Cloneable {
    public static final Integer UNSAVED_ID = new Integer(-1);
    public static final String ID_PROPERTY = "id";

    private static int s_id = 1;

    private Integer m_id;

    public BeanWithId() {
        this(UNSAVED_ID);
    }

    public BeanWithId(Integer id) {
        setId(id);
    }

    void setId(Integer id) {
        m_id = id;
    }

    public Integer getId() {
        return m_id;
    }

    /**
     * Checks if the object has been saved to the database Works because hibernate changes id when
     * the object is saved
     *
     * @return true if the object has never been saved
     */
    public boolean isNew() {
        return UNSAVED_ID.equals(getId());
    }

    public boolean equals(Object o) {
        if (!(o instanceof BeanWithId)) {
            return false;
        }
        BeanWithId other = (BeanWithId) o;
        return getId().equals(other.getId());
    }

    public int hashCode() {
        return m_id.hashCode();
    }

    public void update(BeanWithId object) {
        try {
            Integer saveId = getId();
            BeanUtils.copyProperties(this, object);
            setId(saveId);
        } catch (IllegalAccessException e) {
            throw new RuntimeException(e);
        } catch (InvocationTargetException e) {
            throw new RuntimeException(e);
        }
    }

    /**
     * Special version of clone, replaces beanId with a new one in a cloned object. Do not
     * override duplicate, override clone instead.
     */
    public final BeanWithId duplicate() {
        try {
            BeanWithId clone = (BeanWithId) clone();
            clone.setId(UNSAVED_ID);
            return clone;
        } catch (CloneNotSupportedException e) {
            throw new RuntimeException(e);
        }
    }

    /**
     * Assigns a unique id to a newly created object.
     *
     * For testing only. Most objects are created with id -1 and hibernate sets a proper id. We
     * want to be able to set the id to a unique value in tests.
     *
     * @return the same object - to allow for chaining calls
     */
    public BeanWithId setUniqueId() {
        setId(new Integer(s_id++));
        return this;
    }

    /**
     * Assigns a id to a newly created object.
     *
     * For testing ONLY. Most objects are created with id -1 and hibernate sets a proper id. We
     * want to be able to set the id to a unique value in tests.
     *
     * @return the same object - to allow for chaining calls
     */

    public BeanWithId setUniqueId(int val) {
        setId(new Integer(val));
        return this;
    }

    public static final class BeanToId implements Transformer {
        public Object transform(Object item) {
            BeanWithId bean = (BeanWithId) item;
            return bean.getId();
        }
    }

    public static final class IdToBean implements Transformer {
        private final HibernateTemplate m_template;
        private final Class m_klass;

        public IdToBean(HibernateTemplate template, Class klass) {
            m_template = template;
            m_klass = klass;
        }

        public Object transform(Object input) {
            return m_template.load(m_klass, (Serializable) input);
        }
    }

    /**
     * Implementation of PrimaryKeySource
     */
    public Object getPrimaryKey() {
        return getId();
    }
}
