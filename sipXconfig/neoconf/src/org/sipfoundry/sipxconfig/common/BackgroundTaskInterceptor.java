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

import java.util.Hashtable;

import org.aopalliance.intercept.MethodInterceptor;
import org.aopalliance.intercept.MethodInvocation;
import org.apache.log4j.MDC;

public class BackgroundTaskInterceptor implements MethodInterceptor {
    private final BackgroundTaskQueue m_queue;

    public BackgroundTaskInterceptor() {
        this(new BackgroundTaskQueue());
    }

    public BackgroundTaskInterceptor(BackgroundTaskQueue queue) {
        m_queue = queue;
    }

    public Object invoke(MethodInvocation invocation) throws Throwable {
        InvocationTask task = new InvocationTask(invocation, MDC.getContext());
        m_queue.addTask(task);
        return null;
    }

    /*
     * Starts a new thread to handle the background task. The MDC Hashtable is passed in to allow
     * a logging context to be maintained across threads.
     */
    private static class InvocationTask implements Runnable {
        private final MethodInvocation m_invocation;
        private final Hashtable<String, String> m_logContext;

        public InvocationTask(MethodInvocation invocation, Hashtable<String, String> logContext) {
            m_invocation = invocation;
            m_logContext = logContext;
        }

        public void run() {
            try {
                if (m_logContext != null) {
                    for (String key : m_logContext.keySet()) {
                        MDC.put(key, m_logContext.get(key));
                    }
                }
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
