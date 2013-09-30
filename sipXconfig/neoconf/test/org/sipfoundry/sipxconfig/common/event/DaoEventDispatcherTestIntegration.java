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
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.test.IntegrationTestCase;

public class DaoEventDispatcherTestIntegration extends IntegrationTestCase {
    private DaoEventDispatcher m_dispatcher;
    private DaoEventPublisher m_publisherOrg;
    private CoreContext m_coreContext;

    protected void onSetUpBeforeTransaction() throws Exception {
        super.onSetUpBeforeTransaction();
        m_publisherOrg = null;
        clear();
    }
    
    public void setOnSaveEventDispatcher(DaoEventDispatcher dispatcher) {
        m_dispatcher = dispatcher;
    }

    @Override
    protected void onTearDownAfterTransaction() throws Exception {
        super.onTearDownAfterTransaction();
        // this is called in finally block to restore the application context
        if(m_publisherOrg != null) {
            m_dispatcher.setPublisher(m_publisherOrg);
        }
    }
        	
    public void testOnSaveAspect() throws Exception {
    	User u = new User();
    	u.setUserName("testme");
    	
    	IMocksControl publisherCtrl = EasyMock.createControl();
		DaoEventPublisher publisher = publisherCtrl.createMock(DaoEventPublisher.class);
		publisher.publishBeforeSave(u);
		publisher.publishSave(u);
        publisherCtrl.replay();

        m_publisherOrg = m_dispatcher.getPublisher();
        m_dispatcher.setPublisher(publisher);

        // should generate event
    	m_coreContext.saveUser(u);

        // no event now
    	m_coreContext.clear();
    	
    	publisherCtrl.verify();
    }

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }
}


