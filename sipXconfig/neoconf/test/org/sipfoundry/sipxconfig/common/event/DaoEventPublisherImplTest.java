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

import java.util.HashMap;
import java.util.Map;

import junit.framework.TestCase;

import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.springframework.beans.BeansException;
import org.springframework.context.ApplicationContext;

public class DaoEventPublisherImplTest extends TestCase {

    public void testPublishDelete() {
        Object entity = new Object();

        IMocksControl listenerCtrl = EasyMock.createControl();
        DaoEventListener listener = listenerCtrl.createMock(DaoEventListener.class);
        listener.onDelete(entity);
        listener.onDelete(entity);
        listenerCtrl.replay();

        Map beans = new HashMap();
        beans.put("1", listener);
        beans.put("2", listener);

        IMocksControl appContextCtrl = EasyMock.createControl();
        ApplicationContext appContext = appContextCtrl.createMock(ApplicationContext.class);
        appContext.getBeansOfType(DaoEventListener.class, true, true);
        appContextCtrl.andReturn(beans);
        appContextCtrl.replay();

        DaoEventPublisherImpl impl = new DaoEventPublisherImpl();
        impl.setBeanFactory(appContext);
        impl.publishDelete(entity);

        appContextCtrl.verify();
        listenerCtrl.verify();
    }

    public void testPublishSave() {
        Object entity = new Object();

        IMocksControl listenerCtrl = EasyMock.createControl();
        DaoEventListener listener = listenerCtrl.createMock(DaoEventListener.class);
        listener.onSave(entity);
        listener.onSave(entity);
        listenerCtrl.replay();

        Map beans = new HashMap();
        beans.put("1", listener);
        beans.put("2", listener);

        IMocksControl appContextCtrl = EasyMock.createControl();
        ApplicationContext appContext = appContextCtrl.createMock(ApplicationContext.class);
        appContext.getBeansOfType(DaoEventListener.class, true, true);
        appContextCtrl.andReturn(beans);
        appContextCtrl.replay();

        DaoEventPublisherImpl impl = new DaoEventPublisherImpl();
        impl.setBeanFactory(appContext);
        impl.publishSave(entity);

        appContextCtrl.verify();
        listenerCtrl.verify();
    }

    public void testInitialization() {
        DaoEventPublisherImpl impl = new DaoEventPublisherImpl();

        try {
            impl.publishSave(new Object());
            fail("Should have raised BeansException");
        } catch (BeansException e) {
            // this is expected
        }
    }
}
