/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.common.event;

import java.lang.reflect.Method;

import org.springframework.aop.MethodBeforeAdvice;

public final class DaoEventDispatcher implements MethodBeforeAdvice {

    private static final int ON_DELETE = 1;

    private static final int ON_SAVE = 2;

    private int m_eventType;

    private DaoEventPublisher m_publisher;

    private DaoEventDispatcher(int eventType) {
        m_eventType = eventType;
    }

    public void setPublisher(DaoEventPublisher publisher) {
        m_publisher = publisher;
    }
    
    /* only used in tests */
    DaoEventPublisher getPublisher() {
        return m_publisher;
    }

    public static DaoEventDispatcher createDeleteDispatcher() {
        return new DaoEventDispatcher(ON_DELETE);
    }

    public static DaoEventDispatcher createSaveDispatcher() {
        return new DaoEventDispatcher(ON_SAVE);
    }

    public void before(Method m_, Object[] args, Object target_) throws Throwable {
        if (args.length == 0) {
            // empty arg calls like save() or delete() won't be considered
            return;
        }
        switch (m_eventType) {
        case ON_DELETE:
            m_publisher.publishDelete(args[0]);
            break;
        case ON_SAVE:
            m_publisher.publishSave(args[0]);
            break;
        default:
            throw new RuntimeException("Unknown event type " + m_eventType);
        }
    }
}
