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

import org.aopalliance.intercept.MethodInterceptor;
import org.aopalliance.intercept.MethodInvocation;

public class BackgroundTaskInterceptor implements MethodInterceptor {
    private final BackgroundTaskQueue m_queue; 
    
    public BackgroundTaskInterceptor() {
        this(new BackgroundTaskQueue());
    }
    
    public BackgroundTaskInterceptor(BackgroundTaskQueue queue) {
        m_queue = queue;
    }

    public Object invoke(MethodInvocation invocation) throws Throwable {
        InvocationTask task = new InvocationTask(invocation);
        m_queue.addTask(task);
        return null;
    }

    private static class InvocationTask implements Runnable {
        private final MethodInvocation m_invocation;

        public InvocationTask(MethodInvocation invocation) {
            m_invocation = invocation;
        }

        public void run() {
            try {
                m_invocation.proceed();
            } catch (Throwable e) {
                if (e instanceof RuntimeException) {
                    throw (RuntimeException) e;
                }
                throw new RuntimeException(e);
            }
        }
    }
}
