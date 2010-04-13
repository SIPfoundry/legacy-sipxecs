/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.sip;

import java.lang.reflect.Method;

import org.springframework.aop.MethodBeforeAdvice;

public class InitStackAdvice implements MethodBeforeAdvice {
    private boolean m_init;

    private SipStackBean m_stack;

    public void before(Method method, Object[] args, Object target) throws Throwable {
        if (!m_init) {
            // initialize stack
            m_init = true;
            m_stack.init();
        }
    }

    public void setStack(SipStackBean stack) {
        m_stack = stack;
    }
}
