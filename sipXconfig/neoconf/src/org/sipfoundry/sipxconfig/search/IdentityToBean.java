/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.search;

import org.apache.commons.collections.Transformer;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.common.DataObjectSource;
import org.sipfoundry.sipxconfig.search.BeanAdaptor.Identity;
import org.springframework.orm.hibernate3.HibernateObjectRetrievalFailureException;

/**
 * Tries to load bean from DataObjectSource if the object cannot be loaded catches exception and returns null
 * IdentityToBean
 */
public class IdentityToBean<T> implements Transformer {
    public static final Log LOG = LogFactory.getLog(IdentityToBean.class);

    private DataObjectSource<T> m_source;

    public IdentityToBean(DataObjectSource<T> source) {
        m_source = source;
    }

    public T transform(Object identity) {
        try {
            Identity<T> i = (Identity<T>) identity;
            return m_source.load(i.getBeanClass(), i.getBeanId());
        } catch (HibernateObjectRetrievalFailureException e) {
            LOG.error("Object not found: " + identity, e);
            return null;
        }
    }
}
