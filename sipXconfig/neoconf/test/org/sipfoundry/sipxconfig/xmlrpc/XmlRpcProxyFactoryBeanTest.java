/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.xmlrpc;

import junit.framework.TestCase;

import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.xml.XmlBeanFactory;
import org.springframework.core.io.ClassPathResource;
import org.springframework.core.io.Resource;

public class XmlRpcProxyFactoryBeanTest extends TestCase {
    private Server m_server;

    protected void setUp() throws Exception {
        m_server = new Server();
    }

    protected void tearDown() throws Exception {
        m_server.stop();
    }

    public void testProxy() {
        Resource beans = new ClassPathResource("beans.xml", getClass());
        BeanFactory beanFactory = new XmlBeanFactory(beans);


        TestFunctions testFunctions = (TestFunctions) beanFactory.getBean("testXmlRpcFunctions");

        String result = testFunctions.multiplyTest("ab", 2);
        assertEquals("abab", result);
    }
}
