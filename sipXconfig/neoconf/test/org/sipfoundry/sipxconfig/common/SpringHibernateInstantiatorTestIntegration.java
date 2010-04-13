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

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.IntegrationTestCase;
import org.sipfoundry.sipxconfig.gateway.Gateway;
import org.sipfoundry.sipxconfig.gateway.audiocodes.AudioCodesFxoGateway;
import org.springframework.context.ApplicationContext;

public class SpringHibernateInstantiatorTestIntegration
    extends IntegrationTestCase {

    private SpringHibernateInstantiator m_instantiator;

    /* This method is named init as the superclass has defined the
     * setUp method to be final.  Therefore, each test must explicitly
     * call this method
     */
    protected void init() throws Exception {
        ApplicationContext m_applicationContext = getApplicationContext();
        m_instantiator = new SpringHibernateInstantiator();
        m_instantiator.setBeanFactory(m_applicationContext);
        // to make sure that test are valid
        assertTrue(m_applicationContext.getBeanNamesForType(Gateway.class).length > 1);
    }

    public void testInstantiate() throws Exception {
        init();
        Integer id = new Integer(5);
        BeanWithId bean = (BeanWithId) m_instantiator.instantiate(Gateway.class, id);
        assertSame(Gateway.class, bean.getClass());
        assertSame(id, bean.getId());
        BeanWithId bean2 = (BeanWithId) m_instantiator.instantiate(Gateway.class, id);
        assertNotSame(bean2, bean);
    }

    public void testInstantiateSubclass() throws Exception {
        init();
        Integer id = new Integer(5);
        BeanWithId bean = (BeanWithId) m_instantiator.instantiate(AudioCodesFxoGateway.class, id);
        assertSame(AudioCodesFxoGateway.class, bean.getClass());
        assertSame(id, bean.getId());
    }

    public void testInstantiateUnknown() throws Exception {
        init();
        Integer id = new Integer(5);
        // there is a good chance we will not have StringUtils in beanFactory
        Object bean = m_instantiator.instantiate(StringUtils.class, id);
        assertNull(bean);
        Object bean2 = m_instantiator.instantiate(StringUtils.class, id);
        assertNull(bean2);
    }
}
