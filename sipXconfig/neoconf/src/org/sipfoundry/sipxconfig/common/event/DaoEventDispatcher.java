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

import java.util.Collection;

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
        boolean proceed = true;
        if (method.getArguments().length == 0) {
            // empty arg calls like save() or delete() won't be considered
            // because there is no entity to distinguish event
            proceed = false;
        }

        SupressDaoEvent skip = method.getMethod().getAnnotation(SupressDaoEvent.class);
        if (skip != null) {
            proceed = false;
        }

        if (!proceed) {
            return method.proceed();
        }

        Object entity = method.getArguments()[0];
        Object response;
        switch (m_eventType) {
        case ON_SAVE:

            // opportunity to veto by throwing exception here
            if (entity instanceof Collection) {
                m_publisher.publishBeforeSaveCollection((Collection) entity);
            } else {
                m_publisher.publishBeforeSave(entity);
            }

            response = method.proceed();

            // save happened, live with it now
            if (entity instanceof Collection) {
                m_publisher.publishSaveCollection((Collection) entity);
            } else {
                m_publisher.publishSave(entity);
            }
            break;

        case ON_DELETE:

            // opportunity to veto by throwing exception here
            if (entity instanceof Collection) {
                m_publisher.publishDeleteCollection((Collection) entity);
            } else {
                m_publisher.publishDelete(entity);
            }

            response = method.proceed();

            // delete happened, live with it now
            if (entity instanceof Collection) {
                m_publisher.publishAfterDeleteCollection((Collection) entity);
            } else {
                m_publisher.publishAfterDelete(entity);
            }
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
