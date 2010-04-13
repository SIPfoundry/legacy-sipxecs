/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */

package org.sipfoundry.sipxconfig.login;

import org.sipfoundry.sipxconfig.IntegrationTestCase;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;

public class PrivateUserKeyManagerTestIntegration extends IntegrationTestCase {
    private PrivateUserKeyManager m_manager;

    private CoreContext m_coreContext;

    public void testPrivateKeyForUser() throws Exception {
        loadDataSet("admin/dialplan/seedUser.xml");

        User user = m_coreContext.loadUser(1001);

        assertNull(m_manager.getUserFromPrivateKey("XXX"));

        String key = m_manager.getPrivateKeyForUser(user);

        assertEquals(user, m_manager.getUserFromPrivateKey(key));

        m_coreContext.deleteUser(user);
    }

    public void setPrivateUserKeyManager(PrivateUserKeyManager manager) {
        m_manager = manager;
    }

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }
}
