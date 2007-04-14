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

import junit.framework.TestCase;

import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.sipfoundry.sipxconfig.common.User;

public class DaoEventDispatcherTest extends TestCase {
	
	public void testOnDelete() throws Throwable {
        	User u = new User();
        	u.setUserName("testme");
    
        	IMocksControl publisherCtrl = EasyMock.createControl();
		DaoEventPublisher publisher = publisherCtrl.createMock(DaoEventPublisher.class);
		publisher.publishDelete(u);
		publisherCtrl.replay();
		
		DaoEventDispatcher dispatcher = DaoEventDispatcher.createDeleteDispatcher();
        dispatcher.setPublisher(publisher);
		dispatcher.before(null, new Object[] {u}, null);
		
		publisherCtrl.verify();		
	}

    public void testOnSave() throws Throwable {
        User u = new User();
        u.setUserName("testme");

        IMocksControl publisherCtrl = EasyMock.createControl();
        DaoEventPublisher publisher = publisherCtrl.createMock(DaoEventPublisher.class);
        publisher.publishSave(u);
        publisherCtrl.replay();
        
        DaoEventDispatcher dispatcher = DaoEventDispatcher.createSaveDispatcher();
        dispatcher.setPublisher(publisher);
        dispatcher.before(null, new Object[] {u}, null);
        
        publisherCtrl.verify();     
    }    
}


