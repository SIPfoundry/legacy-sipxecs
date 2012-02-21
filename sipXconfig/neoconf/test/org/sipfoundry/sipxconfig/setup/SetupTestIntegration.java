/*
 * Copyright (C) 2012 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.setup;

import org.sipfoundry.sipxconfig.test.IntegrationTestCase;

public class SetupTestIntegration extends IntegrationTestCase {
    private SetupManager m_setupManager;
    private SetupManagerImpl m_setupManagerImpl;
    
    @Override
    protected void onSetUpBeforeTransaction() throws Exception {
        super.onSetUpBeforeTransaction();
        clear();
        db().execute("insert into setup values ('SetupTestIntegration')");
    }

    public void testSetupIds() {
        assertTrue(m_setupManager.isSetup("SetupTestIntegration"));
        
        String newlySetup = "SetupTestIntegration-newlySetup";
        assertFalse(m_setupManager.isSetup(newlySetup));
        m_setupManager.setSetup(newlySetup);
        assertTrue(m_setupManager.isSetup(newlySetup));
        m_setupManagerImpl.saveSetupIds();
        flush();
        db().queryForInt("select 1 from setup where setup_id = ?", newlySetup);
    }

    public void setSetupManager(SetupManager setupManager) {
        m_setupManager = setupManager;
    }

    public void setSetupManagerImpl(SetupManagerImpl setupManagerImpl) {
        m_setupManagerImpl = setupManagerImpl;
    }
}
