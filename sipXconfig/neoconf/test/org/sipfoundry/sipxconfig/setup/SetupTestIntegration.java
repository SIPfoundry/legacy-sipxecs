/**
 *
 *
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
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
        assertTrue(m_setupManager.isTrue("SetupTestIntegration"));
        
        String newlySetup = "SetupTestIntegration-newlySetup";
        assertFalse(m_setupManager.isTrue(newlySetup));
        m_setupManager.setTrue(newlySetup);
        assertTrue(m_setupManager.isTrue(newlySetup));
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
