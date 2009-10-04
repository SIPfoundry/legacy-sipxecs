/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxconfig.common;

import org.sipfoundry.sipxconfig.IntegrationTestCase;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.permission.PermissionManager;
/**
 * Contains Integration tests. All tests from CoreContextImplTestDb should be moved here
 * and CoreContextImplTestDb should be deleted
 */
public class CoreContextImplTestIntegration extends IntegrationTestCase {
    private CoreContextImpl m_coreContext;
    private PermissionManager m_permissionManager;
    private DomainManager m_domainManager;

    public void testIsImIdUnique() throws Exception {
        loadDataSetXml("common/ClearUsers.xml");
        loadDataSetXml("common/Users.xml");
        //check im id uniqueness for a new user
        User user = new User();
        user.setUniqueId();
        user.setImId("openfire1");
        assertFalse(m_coreContext.isImIdUnique(user));
        user.setImId("openfire22");
        assertTrue(m_coreContext.isImIdUnique(user));
        //check im id uniqueness for an existing user
        User existingUser = m_coreContext.loadUser(1001);
        assertTrue(m_coreContext.isImIdUnique(existingUser));
    }

    public void testApplyImId() throws Exception {
        m_coreContext.setDomainManager(m_domainManager);
        loadDataSetXml("common/ClearUsers.xml");
        User user = new User();
        user.setUserName("201");
        user.setPermissionManager(m_permissionManager);
        user.setSettingTypedValue("im/im-account", true);
        m_coreContext.saveUser(user);
        User savedUser = m_coreContext.loadUserByUserName("201");
        assertEquals("201", savedUser.getImId());
        assertEquals("201", savedUser.getImDisplayName());
        savedUser.setUserName("202");
        savedUser.setImId(null);
        savedUser.setImDisplayName("imDisplayName");
        m_coreContext.saveUser(savedUser);
        User modifiedUser = m_coreContext.loadUserByUserName("202");
        assertEquals("202", modifiedUser.getImId());
        assertEquals("imDisplayName", modifiedUser.getImDisplayName());
    }

    public void setCoreContextImpl(CoreContextImpl coreContext) {
        m_coreContext = coreContext;
    }

    public void setPermissionManager(PermissionManager permissionManager) {
        m_permissionManager = permissionManager;
    }

    public void setDomainManager(DomainManager domainManager) {
        m_domainManager = domainManager;
    }

}
