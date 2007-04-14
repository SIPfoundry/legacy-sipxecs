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

import junit.framework.TestCase;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.gateway.Gateway;
import org.sipfoundry.sipxconfig.gateway.audiocodes.MediantGateway;
import org.springframework.beans.factory.ListableBeanFactory;

public class SpringHibernateInstantiatorTest extends TestCase {

    private SpringHibernateInstantiator m_instantiator;

    protected void setUp() throws Exception {
        ListableBeanFactory context = TestHelper.getApplicationContext();
        m_instantiator = new SpringHibernateInstantiator();
        m_instantiator.setBeanFactory(context);
        // to make sure that test are valid
        assertTrue(context.getBeanNamesForType(Gateway.class).length > 1);
    }

    public void testInstantiate() {
        Integer id = new Integer(5);
        BeanWithId bean = (BeanWithId) m_instantiator.instantiate(Gateway.class, id);
        assertSame(Gateway.class, bean.getClass());
        assertSame(id, bean.getId());
        BeanWithId bean2 = (BeanWithId) m_instantiator.instantiate(Gateway.class, id);
        assertNotSame(bean2, bean);
    }

    public void testInstantiateSubclass() {
        Integer id = new Integer(5);
        BeanWithId bean = (BeanWithId) m_instantiator.instantiate(MediantGateway.class, id);
        assertSame(MediantGateway.class, bean.getClass());
        assertSame(id, bean.getId());
    }

    public void testInstantiateUnknown() {
        Integer id = new Integer(5);
        // there is a good chance we will not have StringUtils in beanFactory
        Object bean = m_instantiator.instantiate(StringUtils.class, id);
        assertNull(bean);
        Object bean2 = m_instantiator.instantiate(StringUtils.class, id);
        assertNull(bean2);
    }
}
