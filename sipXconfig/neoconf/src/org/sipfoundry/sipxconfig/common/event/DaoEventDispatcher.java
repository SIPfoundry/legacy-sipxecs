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

import org.aopalliance.intercept.MethodInterceptor;
import org.aopalliance.intercept.MethodInvocation;

public final class DaoEventDispatcher implements MethodInterceptor {

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

    public Object invoke(final MethodInvocation method) throws Throwable {
        if (method.getArguments().length == 0) {
            // empty arg calls like save() or delete() won't be considered
            // because there is no entity to distinguish event
            return method.proceed();
        }

        Object entity = method.getArguments()[0];
        Object response;
        switch (m_eventType) {
        case ON_SAVE:

            // XCF-768  Call save first to ensure session initializes correctly (whatever
            // correctly means) before sending event that may trigger a "redundant object
            // in session" exception
            response = method.proceed();

            m_publisher.publishSave(entity);
            break;
        case ON_DELETE:
            m_publisher.publishDelete(entity);

            // XCF-768 Delete may have same problem as save, but I wouldn't want to send
            // an event about an object that has already been deleted, especially in case
            // there's a listner that wishes to veto delete.  Until there's a problem or
            // XCF-768 gets resolved once and for all, leave ordering as is.
            response = method.proceed();

            break;
        default:
            throw new RuntimeException("Unknown event type " + m_eventType);
        }
        return response;
    }

    class WrappedException extends RuntimeException {
        WrappedException(Throwable wrapped) {
            super(wrapped);
        }
    }
}
