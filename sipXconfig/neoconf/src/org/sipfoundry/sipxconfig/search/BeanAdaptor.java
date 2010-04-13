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

import java.io.Serializable;

import org.apache.lucene.document.Document;
import org.apache.lucene.index.Term;
import org.hibernate.type.Type;

public interface BeanAdaptor {

    /**
     * @return true if the document should be added to index
     */
    boolean documentFromBean(Document document, Object bean, Serializable id, Object[] state,
            String[] fieldNames, Type[] types);

    Term getIdentityTerm(Object bean, Serializable id);

    Identity getBeanIdentity(Document document);

    boolean indexClass(Document document, Class klass);

    void setIndexedClasses(Class[] indexedClasses);

    public static class Identity<T> {
        private Class<T> m_klass;
        private Serializable m_id;
        private String m_name;
        private String m_description;

        public Identity(Class<T> klass, Serializable id) {
            m_klass = klass;
            m_id = id;
        }

        public Serializable getBeanId() {
            return m_id;
        }

        public Class<T> getBeanClass() {
            return m_klass;
        }

        public String getDescription() {
            return m_description;
        }

        public void setDescription(String description) {
            m_description = description;
        }

        public String getName() {
            return m_name;
        }

        public void setName(String name) {
            m_name = name;
        }
    }
}
