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

import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;

import org.apache.commons.collections.CollectionUtils;

/**
 * BeanId -- uniquely identify a persisted object of type BeanWithId through two properties,
 * its Java class and its database ID.
 */
public class BeanId {
    private Integer m_id;
    private Class m_beanClass;

    public BeanId(Integer id, Class beanClass) {
        m_id = id;
        m_beanClass = beanClass;
    }

    public BeanId(BeanWithId bean) {
        this(bean.getId(), bean.getClass());
    }

    /**
     * Given a Collection of IDs and a Java class, create and return a Collection of BeanIds.
     * Throw an exception if any ID is negative (unsaved object) or is not unique.
     */
    public static Collection createBeanIdCollection(Collection ids, Class beanClass) {
        if (SipxCollectionUtils.safeIsEmpty(ids)) {
            return CollectionUtils.EMPTY_COLLECTION;
        }
        Collection bids = new ArrayList(ids.size());
        Collection idCheck = new ArrayList(ids.size());     // for uniqueness checking
        for (Iterator iter = ids.iterator(); iter.hasNext();) {
            Integer id = (Integer) iter.next();
            if (id == null) {
                throw new IllegalArgumentException("The ID collection contains a null ID");
            }
            int idVal = id.intValue();
            if (idVal < 0) {
                throw new IllegalArgumentException("The ID collection contains a negative ID: "
                        + idVal);
            }
            if (idCheck.contains(id)) {
                throw new IllegalArgumentException(
                        "The ID collection contains this ID more than once: " + idVal);
            }
            idCheck.add(id);
            BeanId bid = new BeanId(id, beanClass);
            bids.add(bid);
        }
        return bids;
    }

    public Class getBeanClass() {
        return m_beanClass;
    }
    public void setBeanClass(Class beanClass) {
        m_beanClass = beanClass;
    }
    public Integer getId() {
        return m_id;
    }
    public void setId(Integer id) {
        m_id = id;
    }

    public boolean isIdOfBean(BeanWithId bean) {
        if (bean == null) {
            return false;
        }
        return getBeanClass().equals(bean.getClass())
                && getId().equals(bean.getId());
    }

    public boolean equals(Object obj) {
        if (obj instanceof BeanId) {
            BeanId bid = (BeanId) obj;
            return bid.getBeanClass().equals(getBeanClass())
                && bid.getId().equals(getId());
        }
        return false;
    }

    public int hashCode() {
        int hashCode = getClass().getName().hashCode() * (getId().intValue() + 1);
        return hashCode;
    }

    public String toString() {
        StringBuffer buf = new StringBuffer(m_beanClass != null
                ? m_beanClass.getName()
                : "null class");
        buf.append(", ID=");
        buf.append(m_id);
        return buf.toString();
    }

}
