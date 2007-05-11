/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.common;

import org.sipfoundry.sipxconfig.IntegrationTestCase;

public class UserTestIntegration extends IntegrationTestCase {    
    private CoreContext m_coreContext;    
    
    public void testLoadUser() {
        loadDataSet("common/TestUserSeed.db.xml");
        int userId = 1000;
        User user = m_coreContext.loadUser(1000);
        assertEquals(userId, user.getPrimaryKey());
        assertEquals(userId, user.getId().intValue());
    }

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }
}
