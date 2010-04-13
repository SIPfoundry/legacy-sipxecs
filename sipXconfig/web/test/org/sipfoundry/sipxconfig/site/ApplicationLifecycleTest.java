/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site;

import junit.framework.TestCase;

import org.apache.tapestry.engine.state.ApplicationStateManager;
import org.easymock.EasyMock;
import org.easymock.IMocksControl;

public class ApplicationLifecycleTest extends TestCase {

    public void testLogout() {
        IMocksControl stateManagerCtrl = EasyMock.createControl();
        ApplicationStateManager stateManager = stateManagerCtrl.createMock(ApplicationStateManager.class);
        stateManager.exists(UserSession.SESSION_NAME);
        stateManagerCtrl.andReturn(true);
        stateManager.get(UserSession.SESSION_NAME);
        stateManagerCtrl.andReturn(new UserSession());
        stateManagerCtrl.replay();

        ApplicationLifecycleImpl life = new ApplicationLifecycleImpl();
        life.setStateManager(stateManager);
        life.logout();
        assertTrue(life.getDiscardSession());

        stateManagerCtrl.verify();
    }
}
