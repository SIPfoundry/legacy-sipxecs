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

import org.springframework.aop.framework.ProxyFactory;

public class BackgroundTaskInterceptorTest extends TestCase {
    private StringBuffer m_buffer;
    private BackgroundTaskQueue m_queue;

    protected void setUp() throws Exception {
        m_buffer = new StringBuffer();
        m_queue = new BackgroundTaskQueue();
    }

    public interface TestInterface {
        void doSomething() throws Exception;
    }

    public void testProxy() throws Exception {
        BackgroundTaskInterceptor interceptor = new BackgroundTaskInterceptor(m_queue);
        m_queue.suspend();

        TestInterface target = new TestInterface() {
            public void doSomething() throws Exception {
                m_buffer.append("x");
            }
        };
        ProxyFactory pf = new ProxyFactory(target);
        pf.addAdvice(interceptor);

        TestInterface proxy = (TestInterface) pf.getProxy();
        proxy.doSomething();
        proxy.doSomething();

        assertEquals("", m_buffer.toString());

        m_queue.resume();

        m_queue.yieldTillEmpty();

        assertEquals("xx", m_buffer.toString());
    }
}
