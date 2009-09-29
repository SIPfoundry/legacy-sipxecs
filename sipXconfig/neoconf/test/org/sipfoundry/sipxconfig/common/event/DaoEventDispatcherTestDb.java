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

import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.sipfoundry.sipxconfig.SipxDatabaseTestCase;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.springframework.context.ApplicationContext;

public class DaoEventDispatcherTestDb extends SipxDatabaseTestCase {

    private DaoEventDispatcher m_dispatcher;
    private DaoEventPublisher m_publisherOrg;
    private CoreContext m_core;


    protected void setUp() throws Exception {
        ApplicationContext app = TestHelper.getApplicationContext();
        m_core = (CoreContext) app.getBean(CoreContext.CONTEXT_BEAN_NAME);
        m_dispatcher = (DaoEventDispatcher)app.getBean("onSaveEventDispatcher");
        TestHelper.cleanInsert("ClearDb.xml");
        m_publisherOrg = null;
    }

    protected void tearDown() throws Exception {
        // this is called in finally block to restore the appliation context
        if(m_publisherOrg != null) {
            m_dispatcher.setPublisher(m_publisherOrg);
        }
    }
        	
    public void testOnSaveAspect() throws Exception {
        	User u = new User();
        	u.setUserName("testme");
        	
        	IMocksControl publisherCtrl = EasyMock.createControl();
		DaoEventPublisher publisher = publisherCtrl.createMock(DaoEventPublisher.class);
		publisher.publishSave(u);
        publisherCtrl.replay();

        m_publisherOrg = m_dispatcher.getPublisher();
        m_dispatcher.setPublisher(publisher);

        // should generate event
    	m_core.saveUser(u);

        // no event now
    	m_core.clear();
    	
    	publisherCtrl.verify();
    }
}


