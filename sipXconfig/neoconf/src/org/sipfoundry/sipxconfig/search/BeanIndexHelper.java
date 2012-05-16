/*
 *
 *
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.search;

import java.io.Serializable;
import java.util.List;

import org.sipfoundry.sipxconfig.common.SpringHibernateInstantiator;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.springframework.orm.hibernate3.support.HibernateDaoSupport;

public class BeanIndexHelper extends HibernateDaoSupport {
    private static final String BEAN_ID = "beanId";
    private SpringHibernateInstantiator m_springInstantiator;

    public void setupIndexProperties(BeanIndexProperties beanIndexProperties, boolean loadObjectFromSession) {
        Object entity = beanIndexProperties.getEntity();
        Serializable id = beanIndexProperties.getId();
        List<String> propertyNames = beanIndexProperties.getPropertyNamesList();
        List<Object> state = beanIndexProperties.getStateList();

        modifyPhoneBeans(entity, id, propertyNames, state, loadObjectFromSession);
    }

    // Replace the beanId in Phone class with the phone's model label that's
    // shown in the UI
    private void modifyPhoneBeans(Object entity, Serializable id, List<String> propertyNames, List<Object> state,
            boolean loadFromSession) {
        if (null != entity && entity instanceof Phone) {
            Phone phone = null;
            if (loadFromSession) {
                phone = (Phone) getHibernateTemplate().get(entity.getClass(), id);
            } else {
                phone = (Phone) entity;
            }
            if (propertyNames.contains(BEAN_ID)) {
                int propertyIndex = propertyNames.indexOf(BEAN_ID);
                state.set(propertyIndex, phone.getModelLabel());
            }
        }
    }

    public void setSpringInstantiator(SpringHibernateInstantiator springInstantiator) {
        // without this, phone beans are not created from spring because phone beans are
        // loaded special in another session
        m_springInstantiator = springInstantiator;
        getHibernateTemplate().setEntityInterceptor(m_springInstantiator);
    }
}
